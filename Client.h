#ifndef CLIENT_H 
#define CLIENT_H

#include <linux/workqueue.h>
#include <net/sock.h>
#include "SocketHandler.h"

#define BUF_SIZE 64

extern struct workqueue_struct *client_wq; // celle la est la workqueue des clients 
struct connection_context {
    struct socket *client_sock;

    struct work_struct work_c;
    struct work_struct cpu_task;
    struct work_struct net_task;

    int mySocket; // pour savoir si on doit fermer la socket ou pas
    
    uint8_t *buffer;
};

void client_handle(struct work_struct *work);


#endif
