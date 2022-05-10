#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#define STACK_SIZE 1024

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

void app_main() {

    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;
    BaseType_t xReturned1;
    TaskHandle_t xHandle1 = NULL;

    xReturned = xTaskCreate(
                    task_10ms,       /* Function that implements the task. */
                    "task_10ms",          /* Text name for the task. */
                    STACK_SIZE,      /* Stack size in words, not bytes. */
                    ( void * ) 1,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,/* Priority at which the task is created. */
                    &xHandle );      /* Used to pass out the created task's handle. */
    xReturned1 = xTaskCreate(
                    task_20ms,       /* Function that implements the task. */
                    "task_20ms",          /* Text name for the task. */
                    STACK_SIZE,      /* Stack size in words, not bytes. */
                    ( void * ) 1,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,/* Priority at which the task is created. */
                    &xHandle1 );      /* Used to pass out the created task's handle. */

    while (1) {
        printf("[fcc_comms] hello world\n");

        // 1 second delay
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}
