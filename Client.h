#ifndef CLIENT_H// On peut utiliser l'alternative ifndef 
#define CLIENT_H

#include <linux/workqueue.h>
#include <net/sock.h>

struct workqueue_struct *client_wq;

void client_handle(struct work_struct *work);


#endif