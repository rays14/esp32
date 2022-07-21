#include "tasking.h"

void taskRunning(struct tasklist_t *ptTaskList) {
    assert(ptTaskList);

    ptTaskList->bRunning = true;
}

void taskNotRunning(struct tasklist_t *ptTaskList) {
    assert(ptTaskList);

    ptTaskList->bRunning = false;
}

void taskDetectOvRun(struct tasklist_t *ptTaskList) {
    assert(ptTaskList);

    if (ptTaskList->bRuning) {
        ptTaskList->u32OvRun++;
        if (ptTaskList->u32OvRun > TASK_OVRUN_FAULT) {
            // SET FAULT
        }
    } else {
        ptTaskList->u32OvRun--;
        if (ptTaskList->u32OvRun < TASK_OVRUN_CLEAR_FAULT) {
            // CLEAR FAULT
        }
    }
}

