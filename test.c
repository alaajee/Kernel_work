#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define BUFFER_SIZE 512
#define NUM_OPERATIONS 3

// Structure pour passer les arguments aux threads
typedef struct {
    int thread_id;
    int sockfd;
} thread_args;

void* client_operations(void* arg) {
    thread_args* args = (thread_args*)arg;
    int i = args->thread_id;
    int sockfd = args->sockfd;
    free(arg);

    char message[128];
    char buffer[BUFFER_SIZE];
    int k;

    // Phase PUT
    for (k = 0; k < NUM_OPERATIONS; k++) {
        snprintf(message, sizeof(message), "put name%d alaa%d\n", k, k);
        printf("[Thread %d] PUT name%d -> alaa%d\n", i, k, k);

        if (send(sockfd, message, strlen(message), 0) < 0) {
            perror("send failed");
            close(sockfd);
            return NULL;
        }

        ssize_t recv_bytes = recv(sockfd, buffer, sizeof(buffer)-1, 0);
        if (recv_bytes <= 0) {
            perror("recv failed");
            close(sockfd);
            return NULL;
        }
        buffer[recv_bytes] = '\0';
        printf("[Thread %d] Received: %s\n", i, buffer);
    }

    // Phase GET
    for (k = 0; k < NUM_OPERATIONS; k++) {
        snprintf(message, sizeof(message), "get name%d\n", k);
        printf("[Thread %d] GET name%d\n", i, k);

        if (send(sockfd, message, strlen(message), 0) < 0) {
            perror("send failed");
            close(sockfd);
            return NULL;
        }

        ssize_t recv_bytes = recv(sockfd, buffer, sizeof(buffer)-1, 0);
        if (recv_bytes <= 0) {
            perror("recv failed");
            close(sockfd);
            return NULL;
        }
        buffer[recv_bytes] = '\0';
        printf("[Thread %d] Received: %s\n", i, buffer);
    }

    close(sockfd);
    return NULL;
}

int main() {
    pthread_t threads[30];
    int num_threads = 20;

    printf("Starting %d client threads...\n", num_threads);

    for (int i = 0; i < num_threads; i++) {
        // Création du socket
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket failed");
            continue;
        }

        // Configuration du serveur
        struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(12345),
            .sin_addr.s_addr = htonl(INADDR_LOOPBACK)
        };

        // Connexion
        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("connect failed");
            close(sockfd);
            continue;
        }

        // Préparation des arguments
        thread_args* args = malloc(sizeof(thread_args));
        args->thread_id = i;
        args->sockfd = sockfd;

        // Création du thread
        if (pthread_create(&threads[i], NULL, client_operations, args) != 0) {
            perror("pthread_create failed");
            free(args);
            close(sockfd);
        }

        usleep(100000); // Délai entre création de threads
    }

    // Attente des threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All operations completed.\n");
    return 0;
}