#define STACK_SIZE 1024

struct tasklist_t {
    TaskHandle_t tHandle;
    TickType_t   tTicks;
};

// Semaphores
SemaphoreHandle_t sem;

// Timers
TimerHandle_t tTimerHandle;
uint32_t      u32TimerId = 1;

void timer_cb(TimerHandle_t timer) {
    // This doesn't make sense because timer is the handle
    // of the timer that called this callback.
    if (pvTimerGetTimerID(timer) == (void *)&u32TimerId) {
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
        } else {
            printf("task_sync_30ms : no sem so delay\n");
            vTaskDelay(1000 /portTICK_PERIOD_MS);
        }
        printf("task_sync_30ms\n");
    }
}

