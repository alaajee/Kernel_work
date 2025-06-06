#ifndef SocketHandler_H// On peut utiliser l'alternative ifndef 
#define SocketHandler_H

#include <net/sock.h>
#include <linux/workqueue.h>
#include "Client.h"

#define MAXPENDING 5
#define PORT 11000

extern struct socket *listen_socket ; // une variable pour socket d'ecoute 
extern struct workqueue_struct *client_wq;
typedef struct client_work {
    struct socket* client_sock ;
    struct work_struct work_c;
}client_work;
extern struct client_work *c_work;


int create_Serveur(void);
int handle_Socket(void *data);


#endif