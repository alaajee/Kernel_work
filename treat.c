#include "treat.h"
#include "operation.h"

int treat_task(uint8_t *buf){

    struct work_struct cpu_work;

    // we begin with work_cpu 

    INIT_WORK(&cpu_work , work_cpu);

    queue_work(task_wq, &cpu_work);
    
    return 0;
}

EXPORT_SYMBOL(treat_task);
MODULE_LICENSE("GPL");