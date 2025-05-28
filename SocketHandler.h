#pragma once 
#include <net/sock.h>

struct socket *listen_socket ; // une variable pour socket d'ecoute 


int create_Serveur(void);
int handle_Socket(void);
