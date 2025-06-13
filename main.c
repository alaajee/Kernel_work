#include <linux/module.h> 
#include <linux/printk.h> // for the macro expansion 
#include <linux/init.h> /* Needed for the macros ( module_init and module_exit) */
#include <linux/kernel.h>
#include "SocketHandler.h"
#include <net/sock.h>


int n;
int PORT;
module_param(n, int, 0);
module_param(PORT, int, 0);

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
    client_wq = alloc_workqueue("clients_wq", WQ_UNBOUND, 4);
    task_wq = alloc_workqueue("task_wq",WQ_UNBOUND , 3); // peut etre n ? 

    if (!client_wq || !task_wq) {
        pr_err("Failed to allocate workqueues\n");
        if (client_wq)
            destroy_workqueue(client_wq);
        if (task_wq)
            destroy_workqueue(task_wq);
        return -ENOMEM;
    }

    return create_Serveur();
}

static void __exit my_module_exit(void)
{
    destroy_workqueue(client_wq);
    destroy_workqueue(task_wq);
    if (listen_socket)
        sock_release(listen_socket);
    if (server_thread){
        kthread_stop(server_thread);
    }
    printk(KERN_INFO "Socket libéré.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
