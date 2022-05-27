// Make the 10ms task run off a timer that fires every 10ms
// instead of a task-delay. All other tasks should run off
// this task. Need a task-list to hold sync task handles
// times.
// idf.py set-target esp32c3
// idf.py build flash monitor
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#define STACK_SIZE 1024

// Brint in the task list and add in Overrun.

// Later do this
#define NUM_TASKS           3
#define LEN(x)              (sizeof(x) / sizeof(&x[0]))
#define MAX_GLOBAL_TICKS    100

struct taskitem_t {
    TaskHandle_t       tTaskHandle;
    TickType_t 	       tTicks;
    SemaphoreHandle_t  tSemHandle;
    uint32_t           u32OverrunCount;
    bool               bRunning;
};
struct tasklist_t {
    TickType_t        tGlobalTicks;
    struct taskitem_t list[NUM_TASKS];
};
struct tasklist_t taskList;

void taskInit(struct tasklist_t * ptTaskList) {
    for (uint32_t i = 0; i < LEN(ptTaskList->list); i++) {
            ptTaskList->list[i].tTaskHandle        = 0;
            ptTaskList->list[i].tTicks             = 0;
            ptTaskList->list[i].tSemHandle         = 0;
            ptTaskList->list[i].u32OverrunCount    = 0;
            ptTaskList->list[i].bRunning           = false;
    }
}

int32_t taskAdd (
    struct tasklist_t * ptTaskList, 
    TaskHandle_t        tTaskHandle,
    TickType_t          tTicks,
    SemaphoreHandle_t   tSemHandle) {
    assert(ptTaskList);

    bool bAssigned = false;
    int32_t val = 0;
    ptTaskList->tGlobalTicks = 0;
    for (uint32_t i = 0; i < LEN(ptTaskList->list); i++) {
        if (ptTaskList->list[i].tTaskHandle == NULL) {
            ptTaskList->list[i].tTaskHandle        = tTaskHandle;
            ptTaskList->list[i].tTicks             = tTicks;
            ptTaskList->list[i].tSemHandle         = tSemHandle;
            ptTaskList->list[i].u32OverrunCount    = 0;
            ptTaskList->list[i].bRunning           = false;
            bAssigned                              = true;
        }
    }
    val = (bAssigned) ? 0 : -1;
    return val;
}

void taskRunning(struct taskitem_t *ptTaskItem) {
    assert(ptTaskItem);
    ptTaskItem->bRunning = true;
}

void taskNotRunning(struct taskitem_t *ptTaskItem) {
    assert(ptTaskItem);
    ptTaskItem->bRunning = false;
}

void taskSched(struct tasklist_t *ptTaskList) {
    assert(ptTaskList);

    ptTaskList->tGlobalTicks++;
    if (ptTaskList->tGlobalTicks > MAX_GLOBAL_TICKS) {
        ptTaskList->tGlobalTicks = 0;
    }

    for (uint32_t i = 0; i < LEN(ptTaskList); i++) {
        if (ptTaskList->list[i].tTaskHandle) {
            if ((ptTaskList->tGlobalTicks % ptTaskList->list[i].tTicks) == 0) {
                // Check for overrun
                // Give task semaphore
                if (ptTaskList->list[i].tSemHandle) {
                    xSemaphoreGive(ptTaskList->list[i].tSemHandle);
                } else {
                    // Error
                }
            }
        }
    }

} 

// Semaphores
SemaphoreHandle_t sem;

// Timers
TimerHandle_t tTimerHandle;
uint32_t      u32TimerID = 1;

void timer_cb(TimerHandle_t otTimerHandle) {
    // This doesn't make sense because timer is the handle
    // of the timer that called this callback.
    if (pvTimerGetTimerID(otTimerHandle) == (void *)&u32TimerID) {
        printf("timer_cb : corrent timer-id\n");
    }

    if (sem != NULL) {
            xSemaphoreGive(sem);
    }
}

void task_10ms(void *pvParam) {
    while (1) {
        printf("task_10ms run\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void task_20ms(void *pvParam) {
    while (1) {
        printf("task_20ms run\n");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void task_sync_30ms(void *pvParam) {
    while (1) {
        if (sem != NULL) {
            printf("task_sync_30ms : sem available taking it\n");
            // --------------------------
            // If there sem was created 
            // then use it else just use 
            // plain old delay.
            // --------------------------
            xSemaphoreTake(sem, portMAX_DELAY);
            // Task schedule here
        } else {
            printf("task_sync_30ms : no sem so delay\n");
            vTaskDelay(1000 /portTICK_PERIOD_MS);
        }
        printf("task_sync_30ms\n");
    }
}

void app_main() {
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;
    BaseType_t xReturned1;
    TaskHandle_t xHandle1 = NULL;
    BaseType_t xReturned2;
    TaskHandle_t xHandle2 = NULL;

    // Create the semaphores/timers/queues etc
    sem = xSemaphoreCreateBinary();    
    if (sem == NULL) {
        printf("[ERROR] sem : not enough space to create semaphore\n");
    }

    tTimerHandle = xTimerCreate("timer", pdMS_TO_TICKS(3000), pdTRUE, (void *)&u32TimerID, timer_cb);
    if (tTimerHandle == NULL) {
        printf("[ERROR] timer : not enough space to create timer\n");
    }

    xReturned = xTaskCreate (
                    task_10ms,          // Function that implements the task.
                    "task_10ms",        // Text name for the task.
                    STACK_SIZE,         // Stack size in words, not bytes.
                    ( void * ) 1,       // Parameter passed into the task.
                    tskIDLE_PRIORITY,   // Priority at which the task is created.
                    &xHandle);         // Used to pass out the created task's handle.

    xReturned1 = xTaskCreate (
                    task_20ms,          // Function that implements the task.
                    "task_20ms",        // Text name for the task.
                    STACK_SIZE,         // Stack size in words, not bytes.
                    ( void * ) 1,       // Parameter passed into the task.
                    tskIDLE_PRIORITY,   // Priority at which the task is created.
                    &xHandle1);        // Used to pass out the created task's handle.

    xReturned2 = xTaskCreate (
                    task_sync_30ms,     // Function that implements the task.
                    "task_sync_30ms",   // Text name for the task.
                    STACK_SIZE,         // Stack size in words, not bytes.
                    ( void * ) 1,       // Parameter passed into the task.
                    tskIDLE_PRIORITY,   // Priority at which the task is created.
                    &xHandle2);        // Used to pass out the created task's handle.

    (void)xReturned;
    (void)xReturned1;
    (void)xReturned2;

    xTimerStart(tTimerHandle, 0);

    while (1) {
        printf("[fcc_comms] hello world\n");

        // 1 second delay
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
