#include <linux/module.h> 
#include <linux/printk.h> // for the macro expansion 
#include <linux/init.h> /* Needed for the macros ( module_init and module_exit) */
#include <linux/kernel.h>
#include <net/sock.h>
#include "SocketHandler.h"
#include <linux/kthread.h>


struct socket *listen_socket;
static struct task_struct *server_thread;

int create_Serveur(void){
    // Creer le serveur de socket

    int serv_socket = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &listen_socket);
    
    if (serv_socket < 0){
        printk("Echec creation serveur de socket \n");
        return serv_socket;
    }

    // Preparer le port et l'adresse ip 
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); // ADRESSE 0
    serv_addr.sin_family = AF_INET; // Format IPv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Adresse IP
    serv_addr.sin_port = htons(PORT); // Port TCP

    // Associer l'adresse IP+ port TCP au socket
    serv_socket = kernel_bind(listen_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (serv_socket < 0){
        printk("Echec bind()\n");
        sock_release(listen_socket);
        return serv_socket;
    }

    // Demander l’autorisation au système d’accepter les demandes de connexion
    serv_socket = kernel_listen(listen_socket, MAXPENDING); 
    if (serv_socket < 0){
        printk("Echec listen()\n");
        sock_release(listen_socket);
        return serv_socket;
    }

    server_thread = kthread_run(handle_Socket, client_wq, "socket_server"); // un thread pour accepter les connexions 
    if (IS_ERR(server_thread)) {
        printk(KERN_ERR "Erreur création du thread serveur\n");
        return PTR_ERR(server_thread);
    }
    return 0;
}


int handle_Socket(void *data) {
    struct workqueue_struct *client_wq = (struct workqueue_struct *)data;
    while (!kthread_should_stop()) {
        struct socket *new_client = NULL;
        int error = kernel_accept(listen_socket, &new_client, 0);
        if (error < 0) {
            printk(KERN_ERR "Echec accept()\n");
            continue;
        }

        printk(KERN_INFO "Client connecté !\n");

        struct connection_context *cw = kmalloc(sizeof(struct client_work), GFP_KERNEL);
        if (!cw) {
            printk(KERN_ERR "Erreur allocation tâche client\n");
            sock_release(new_client);
            continue;
        }

        cw->client_sock = new_client;

        // cw->work : celle ci va permettre d'initialiser la tache.
        INIT_WORK(&cw->work_c, client_handle);

        // ceci permet d'ajouter la tache à la wq client_wq et des qu'un thread se libere elle va s'executer !! 
        queue_work(client_wq, &cw->work_c);
    }

    return 0;
}


EXPORT_SYMBOL(create_Serveur); 
MODULE_LICENSE("GPL");


