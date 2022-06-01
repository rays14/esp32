#include "tasking.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

static        SemaphoreHandle_t tSchedulerSemHandle;
static        TimerHandle_t     tSchedulerTimerHandle;
static        TaskHandle_t      tSchedulerTaskHandle;
static        uint32_t          u32SchedulerTimerID = 1;

static void taskSchedule(struct tasklist_t *ptTaskList);
static void taskSchedulerFunc(void *pvParam);
static void timerCb(TimerHandle_t tSchedulerTimerHandle);

void taskInit(struct tasklist_t * ptTaskList) {
    assert(ptTaskList);

    // Initialize the tasklist with zero tasks.
    for (uint32_t i = 0; i < NUM_TASKS; i++) {
        ptTaskList->list[i].tTaskHandle     = 0; 
        ptTaskList->list[i].tTicks          = 0;
        ptTaskList->list[i].tSemHandle      = 0;
        ptTaskList->list[i].u32OverrunCount = 0;
        ptTaskList->list[i].bRunning        = false;
    }

    // Initialize the scheduling-semaphore.
    tSchedulerSemHandle = xSemaphoreCreateBinary();    
    if (tSchedulerSemHandle == NULL) {
        printf("[ERROR] taskInit : Unable to create tSchedulerSemHandle %s:%d\n", __FILE__, __LINE__);
        assert(0);
    }

    // Initialize the scheduling-timer.
    tSchedulerTimerHandle = xTimerCreate("timer1", 
            pdMS_TO_TICKS(SCHED_TIMER_MS), pdTRUE, (void *)&u32SchedulerTimerID, timerCb);

    if (tSchedulerTimerHandle == NULL) {
        printf("[ERROR] taskInit : Unable to create tSchedulerTimerHandle\n");
    }

    // Initialize the scheduling-task.
    int xRet = xTaskCreate (taskSchedulerFunc,              // Function that implements the task.
                            "taskSchedulerFunc",            // Text name for the task.
                            TASK_STACK_SIZE,                // Stack size in words, not bytes.
                            (void *)ptTaskList,             // Parameter passed into the task.
                            tskIDLE_PRIORITY,               // Priority at which the task is created.
                            &tSchedulerTaskHandle);
    (void)xRet;
}

int32_t taskAdd (
    struct tasklist_t *ptTaskList, 
    char              *pcTaskName,
    void             (*pfTask)(void *pvParam),
    uint32_t           u32Ms) {

    assert(ptTaskList);
    assert(pcTaskName);
    assert(pfTask);

    bool    bAssigned = false;
    int32_t val       = 0;

    ptTaskList->tGlobalTicks = 0;
    for (uint32_t i = 0; i < LEN(ptTaskList->list); i++) {
        if (ptTaskList->list[i].tTaskHandle == NULL) {
            int xRet = xTaskCreate (pfTask,                         // Function that implements the task.
                                    pcTaskName,                     // Text name for the task.
                                    TASK_STACK_SIZE,                // Stack size in words, not bytes.
                                    (void *)&(ptTaskList->list[i]), // Parameter passed into the task.
                                    tskIDLE_PRIORITY,               // Priority at which the task is created.
                                    &(ptTaskList->list[i].tTaskHandle));
            (void)xRet;

            ptTaskList->list[i].tTicks             = pdMS_TO_TICKS(u32Ms);
            ptTaskList->list[i].tSemHandle         = xSemaphoreCreateBinary();
            ptTaskList->list[i].u32OverrunCount    = 0;
            ptTaskList->list[i].bRunning           = false;
            bAssigned                              = true;

            assert(ptTaskList->list[i].tSemHandle);

            break;
        }
    }

    val = (bAssigned) ? 0 : -1;

    return val;
}

void taskSchedulerStart (void) {

    // Start the scheduling tasks timer.
    xTimerStart(tSchedulerTimerHandle, 0);
}

void taskRunning(struct taskitem_t *ptTaskItem) {
    assert(ptTaskItem);

    ptTaskItem->bRunning = true;
}

void taskNotRunning(struct taskitem_t *ptTaskItem) {
    assert(ptTaskItem);

    ptTaskItem->bRunning = false;
}

///////////////////////////////////////////////////////////////////////////////
// taskSchedule (Static)
//     Goes through the task list and checks to see if any task is
//     ready to run. If so it release the given task. It also checks
//     for task overruns.
///////////////////////////////////////////////////////////////////////////////
static void taskSchedule(struct tasklist_t *ptTaskList) {
    assert(ptTaskList);

    // Increment global ticks
    ptTaskList->tGlobalTicks++;
    if (ptTaskList->tGlobalTicks >= MAX_GLOBAL_TICKS) {
        ptTaskList->tGlobalTicks = 0;
    }

    for (uint32_t i = 0; i < NUM_TASKS; i++) {
        if (ptTaskList->list[i].tTaskHandle) {
            printf("[NOTE] taskSchedule : task scheduled\n");
            if ((ptTaskList->tGlobalTicks % ptTaskList->list[i].tTicks) == 0) {
                // Check for overrun
                if (ptTaskList->list[i].bRunning) {
                    if (ptTaskList->list[i].u32OverrunCount < MAX_OVERRUN) {
                        ptTaskList->list[i].u32OverrunCount++;
                    }
                }

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

///////////////////////////////////////////////////////////////////////////////
// taskTimerCb (Static)
//     This timer fires every 5ms and calls taskScheduler.
//
///////////////////////////////////////////////////////////////////////////////
static void timerCb(TimerHandle_t tSchedulerTimerHandle) {
    // This doesn't make sense because timer is the handle
    // of the timer that called this callback.
    if (pvTimerGetTimerID(tSchedulerTimerHandle) == (void *)&u32SchedulerTimerID) {
        printf("[NOTE] timerCb : correct timer-id\n");
    }

    if (tSchedulerSemHandle) {
        xSemaphoreGive(tSchedulerSemHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////
// taskScheduler (Static)
//     This is the main scheduling task. All other tasks
//     are scheduled from this task.
//     This task receives a timer callback at 5ms rate.
///////////////////////////////////////////////////////////////////////////////
static void taskSchedulerFunc(void *pvParam) {
    struct tasklist_t *taskList = (struct tasklist_t *)pvParam;
    while (1) {
        printf("[NOTE] taskSchedulerFunc : Running\n");
        // Blocking call
        xSemaphoreTake(tSchedulerSemHandle, portMAX_DELAY);
        taskSchedule(taskList);
    }
}



