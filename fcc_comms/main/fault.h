#ifndef _FAULT_H_
#define _FAULT_H_ 1

struct fault_item_t {
    bool bVal;
    uint32_t u32Count;
    uint32_t u32HiVal;
    uint32_t u32LoVal;
};

struct fault_t {
    struct fault_item_t tFaultTask10msOv;
    struct fault_item_t tFaultTask20msOv;
};

#endif /* _FAULT_H_ */
