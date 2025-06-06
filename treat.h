#ifndef TREAT_H
#define TREAT_H

#include "Client.h"
#include "operation.h"

extern struct workqueue_struct *task_wq;

int treat_task(uint8_t *buf);

#endif