#ifndef _TASKING_H_
#define _TASKING_H_ 1
struct tasklist_t {
    TaskHandle_t tHandle;
    TickType_t 	 tTicks;
    uint32_t     u32OvCnt;
    bool         bRunning;
};

void taskRunning(struct tasklist_t *ptTaskList);
void taskNotRunning(struct tasklist_t *ptTaskList);
void taskDetectOvRun(struct tasklist_t *ptTaskList);

#endif /* _TASKING_H_ */

