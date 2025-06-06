#ifndef CLIENT_H 
#define CLIENT_H

#include <linux/workqueue.h>
#include <net/sock.h>
#include "SocketHandler.h"

#define BUF_SIZE 64

extern struct workqueue_struct *client_wq; // celle la est la workqueue des clients 
extern struct cpu_task {
    struct work_struct work;
     struct socket *client_sock;
};

void client_handle(struct work_struct *work);


#endif
