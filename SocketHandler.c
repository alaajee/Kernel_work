#include <linux/module.h> 
#include <linux/printk.h> // for the macro expansion 
#include <linux/init.h> /* Needed for the macros ( module_init and module_exit) */
#include <linux/kernel.h>
#include <net/sock.h>
#include "SocketHandler.h"
#include "Client.h"

#define MAXPENDING 5


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
    return serv_socket;
}

int handle_Socket(struct socket *client_socket) {
    // TODO : handle correctly the socket _ add the data to the wq ! 
    // Soit une boucle while ou bien accept et ils sont liés ? 
    int error = kernel_accept(&listen_socket , &client_socket , 0);
    if (error < 0){
        printk("Echec accept()\n");
        sock_release(listen_socket);
        return ret;
    }  
    printk("Client connecté !\n");
    client_handle(client_socket);
}


MODULE_LICENSE("GPL");


