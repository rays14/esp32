#ifndef _TASKING_H_
#define _TASKING_H_ 1

#include <stdio.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#define TASK_STACK_SIZE 1024

// Later do this
#define NUM_TASKS           3
#define LEN(x)              (sizeof(x) / sizeof(&x[0]))
#define MAX_GLOBAL_TICKS    100 /* LCM for task scheduling */
#define MAX_OVERRUN         10  /* Max overruns for a task */
#define SCHED_TIMER_MS      10  /* This needs to be 10 or bigger. */


struct taskitem_t {
    TaskHandle_t       tTaskHandle;
    TickType_t         tTicks;
    SemaphoreHandle_t  tSemHandle;
    uint32_t           u32OverrunCount;
    bool               bRunning;
};

struct tasklist_t {
    TickType_t        tGlobalTicks;
    struct taskitem_t list[NUM_TASKS];
};

void taskInit(struct tasklist_t *ptTaskList);

int32_t taskAdd (
    struct tasklist_t *ptTaskList, 
    char              *pcTaskName,
    void             (*pfTask)(void *pvParam),
    uint32_t           u32Ms);

void taskSchedulerStart (void);

void taskRunning(struct taskitem_t *ptTaskItem);

void taskNotRunning(struct taskitem_t *ptTaskItem);

#endif /* _TASKING_H_ */

