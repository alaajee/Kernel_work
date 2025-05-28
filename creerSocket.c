// Ceci en userspace 

// Creer le serveur de socket
int serv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if (serv_socket < 0){
    perror("Echec creation serveur de socket avec socket()\n");
    exit(EXIT_FAILURE);
}

// Creer l'adresse du serveur de socket = adresse IP et port TCP
struct sockaddr_in serv_addr;
memset(&serv_addr, 0, sizeof(serv_addr));
serv_addr.sin_family = AF_INET; // Format IPv4
serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Adresse IP
serv_addr.sin_port = htons(11000); // Port TCP
// Associer l'adresse IP+ port TCP au socket
if (bind(serv_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    perror("Echec bind()\n");
    exit(EXIT_FAILURE);
}
// Demander l'autorisation au système d'écouter des connexions
int MAXPENDING = 5; // nb max de connexions en attente
if (listen(serv_socket, MAXPENDING) < 0){
    perror("Echec listen()\n");
    exit(EXIT_FAILURE);
}