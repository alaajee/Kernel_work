#include <linux/module.h> 
#include <linux/printk.h> // for the macro expansion 
#include <linux/init.h> /* Needed for the macros ( module_init and module_exit) */
#include <linux/kernel.h>
#include <net/sock.h>
#include "SocketHandler.h"

int create_Serveur(void){
    // Creer le serveur de socket
    int serv_socket = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &listen_socket);
    
    if (serv_socket < 0){
        printk("Echec creation serveur de socket \n");
        return serv_socket;
    }

    // Preparer le port et l'adresse ip 
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // Format IPv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Adresse IP
    serv_addr.sin_port = htons(11000); // Port TCP

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

    return 0;
}


int handle_Socket(struct workqueue_struct *client_wq) {
    while (!kthread_should_stop()) {
        struct socket *new_client = NULL;
        int error = kernel_accept(listen_socket, &new_client, 0);
        if (error < 0) {
            printk(KERN_ERR "Echec accept()\n");
            continue;
        }

        printk(KERN_INFO "Client connecté !\n");

        struct client_work *cw = kmalloc(sizeof(struct client_work), GFP_KERNEL);
        if (!cw) {
            printk(KERN_ERR "Erreur allocation tâche client\n");
            sock_release(new_client);
            continue;
        }

        cw->client_sock = new_client;

        // cw_>work : celle ci va permettre d'executer la tache .
        INIT_WORK(&cw->work, client_handle);
        queue_work(client_wq, &cw->work);
    }

    return 0;
}


EXPORT_SYMBOL(create_Serveur); 
MODULE_LICENSE("GPL");


