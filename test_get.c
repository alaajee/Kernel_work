#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define BUFFER_SIZE 400
#define NUM_OPERATIONS 5

// Structure pour passer les arguments aux threads
typedef struct {
    int thread_id;
    int sockfd;
} thread_args;

static inline uint64_t now_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}


void* client_operations(void* arg) {
    thread_args* args = (thread_args*)arg;
    int i = args->thread_id;
    int sockfd = args->sockfd;
    free(arg);

    char message[128];
    char buffer[BUFFER_SIZE];
    int k;

    uint64_t start_time , end_time, latency_time;

    // Phase PUT
    for (k = 0; k < NUM_OPERATIONS; k++) {
        snprintf(message, sizeof(message), "put name%d alaa%d\n", k, k);
        printf("[Thread %d] PUT name%d -> alaa%d\n", i, k, k);
        start_time = now_us();
        if (send(sockfd, message, strlen(message), 0) < 0) {
            perror("send failed");
            close(sockfd);
            return NULL;
        }

        ssize_t recv_bytes = recv(sockfd, buffer, sizeof(buffer)-2, 0);
        if (recv_bytes <= 0) {
            perror("recv failed");
            close(sockfd);
            return NULL;
        }
        end_time = now_us();
        latency_time = end_time - start_time;
        buffer[recv_bytes] = '\0';
        // printf("[Thread %d] Received: %s\n", i, buffer);
        printf("[Thread %d] Received: %s | Latency: %lu us\n", i, buffer, latency_time);
    }

    // sleep(1);
    // // Phase GET
    // for (k = 0; k < 1; k++) {
    //     snprintf(message, sizeof(message), "get name%d\n", k);
    //     printf("[Thread %d] GET name%d\n", i, k);
        
    //     start_time = now_us();

    //     if (send(sockfd, message, strlen(message), 0) < 0) {
    //         perror("send failed");
    //         close(sockfd);
    //         return NULL;
    //     }
        
    //     memset(buffer, 0, sizeof(buffer));
    //     int total = 0;
    //     while (1) {
    //         ssize_t recv_bytes = recv(sockfd, buffer + total, sizeof(buffer) - total - 1, 0);
    //         if (recv_bytes <= 0) {
    //             perror("recv failed");
    //             close(sockfd);
    //             return NULL;
    //         }
    //         total += recv_bytes;
    //         buffer[total] = '\0';

    //         // On a reçu une ligne complète
    //         if (strchr(buffer, '\n') != NULL) {
    //             break;
    //         }
    //     }

        
       
    //     // printf("[Thread %d] Received: %s\n", i, buffer);
    //     end_time = now_us();
    //     latency_time = end_time - start_time;
    //     printf("[Thread %d] Received: %s | Latency: %lu us\n", i, buffer, latency_time);
    // }

    // close(sockfd);
    shutdown(sockfd, SHUT_WR);
    return NULL;
}

int main() {
    pthread_t threads[101];
    int num_threads = 5;

    uint64_t start_time = now_us();

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
            .sin_family = AF_INET, // IPv4
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
        

        // usleep(10000); // Délai entre création de threads
    }

    // Attente des threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        
    }

    uint64_t end_time = now_us();
    double elapsed_sec = (end_time - start_time) / 1e6;

    int total_operations = num_threads * 5 ; // 5 = PUT + GET phases

    double throughput = total_operations / elapsed_sec;

    printf("All operations completed.\n");
    printf("Elapsed time: %.3f seconds\n", elapsed_sec);
    printf("Throughput: %.2f operations per second\n", throughput);
    return 0;
}