#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFFER_SIZE 512

void* send_and_receive(void* arg) {
    int i = *(int*)arg;
    free(arg);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        pthread_exit(NULL);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; //IPV4
    server_addr.sin_port = htons(11000);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Localhost

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        pthread_exit(NULL);
    }

    // Writing a message to the server
    char message[128];
    snprintf(message, sizeof(message), "Hello from user program! #%d", i);
    printf("[Thread %d] Sending: %s\n", i, message);

    for (int j = 0 ; j < 3; j++){
        ssize_t sent_bytes = send(sockfd, message, strlen(message), 0);
        if (sent_bytes < 0) {
            perror("send");
            close(sockfd);
            pthread_exit(NULL);
        }
        printf("[Thread %d] Message sent successfully (%zd bytes)\n", i, sent_bytes);

         // Réception de la réponse
        char buffer[BUFFER_SIZE];
        ssize_t recv_bytes = recv(sockfd, buffer, sizeof(buffer)-1, 0);
        if (recv_bytes < 0) {
            perror("recv");
        } else if (recv_bytes == 0) {
            printf("[Thread %d] Server closed connection\n", i);
        } else {
            buffer[recv_bytes] = '\0';
            printf("[Thread %d] Received: %s\n", i, buffer);
        }
    }
        
        
    
    
    close(sockfd);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[5];

    for (int i = 0; i < 5; i++) {
        int* arg = malloc(sizeof(*arg));
        *arg = i;
        if (pthread_create(&threads[i], NULL, send_and_receive, arg) != 0) {
            perror("pthread_create");
            free(arg);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All messages sent and responses received.\n");
    return 0;
}

// gsettings set org.gnome.desktop.input-sources sources "[('xkb', 'fr')]" 