#ifndef OP_H
#define OP_H

#include "Client.h"

#define MAX_LEN 100

extern int n;

void work_cpu(struct work_struct *cpu_work);
void work_net(struct work_struct *net_work);
void work_store(struct work_struct *store_work);
void work_get(struct work_struct *get_work);
#endif