#ifndef SocketHandler_H// On peut utiliser l'alternative ifndef 
#define SocketHandler_H

#include <net/sock.h>
#include <linux/workqueue.h>
#include "Client.h"

#define MAXPENDING 5

struct socket *listen_socket ; // une variable pour socket d'ecoute 
struct workqueue_struct *client_wq;
typedef struct client_work {
    struct socket* client_sock ;
    struct work_struct work;
}client_work;

int create_Serveur(void);
int handle_Socket(struct workqueue_struct *client_wq);

#endif