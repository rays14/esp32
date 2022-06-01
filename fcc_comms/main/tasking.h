#ifndef _TASKING_H_
#define _TASKING_H_ 1

#define TASK_STACK_SIZE 1024

// Later do this
#define NUM_TASKS           3
#define LEN(x)              (sizeof(x) / sizeof(&x[0]))
#define MAX_GLOBAL_TICKS    100
#define MAX_OVERRUN         10
#define SCHED_TIMER_MS      5


struct taskitem_t {
    TaskHandle_t       tTaskHandle;
    uint32_t           u32Ms;
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
    TickType_t         tTicks);

void taskSchedulerStart (void);

void taskRunning(struct taskitem_t *ptTaskItem);

void taskNotRunning(struct taskitem_t *ptTaskItem);

#endif /* _TASKING_H_ */

