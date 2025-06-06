#include <linux/module.h> 
#include <linux/printk.h> // for the macro expansion 
#include <linux/init.h> /* Needed for the macros ( module_init and module_exit) */
#include <linux/kernel.h>
#include "SocketHandler.h"
#include <net/sock.h>

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Jenninea"); 
MODULE_DESCRIPTION("Simple traitement des socket");

/*
    [ Réception réseau (client) ]
               │
               ▼
        [ Traitement CPU ]
               │
               ▼
   [ Réponse réseau (client) ]
*/

struct workqueue_struct *client_wq;
struct workqueue_struct *task_wq;

static int __init my_module_init(void)
{
    // Prendre en considération les erreurs ! 
    // CHECK THE FLAG WQ_UNBOUND
    client_wq = alloc_workqueue("clients_wq", WQ_UNBOUND, 0);
    task_wq = alloc_workqueue("task_wq",WQ_UNBOUND , 0);

    return create_Serveur();
}

static void __exit my_module_exit(void)
{
   // if (listen_socket)
    //    sock_release(listen_socket);
    printk(KERN_INFO "Socket libéré.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
