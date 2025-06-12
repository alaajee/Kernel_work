#ifndef OP_H
#define OP_H

#include "Client.h"

#define MAX_LEN 100

extern int n;

void work_cpu(struct work_struct *cpu_work);
void net_cpu(struct work_struct *cpu_work);

#endif