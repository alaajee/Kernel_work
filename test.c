#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>

#define BUFFER_SIZE 512
int count = 0;

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
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        pthread_exit(NULL);
    }

    int k = 0;
    bool put = true;
    char message[128];

    for (int j = 0 ; j < 2; j++) {
        if (put) {
            snprintf(message, sizeof(message), "put name%d alaa%d\n", k, k);
            put = false;
            printf("[Thread %d] PUT name%d -> alaa%d\n", i, k, k);
            count++;
            ssize_t sent_bytes = send(sockfd, message, strlen(message), 0);
            if (sent_bytes < 0) {
                perror("send");
                close(sockfd);
                pthread_exit(NULL);
            }

            char buffer[BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer));
            ssize_t recv_bytes = recv(sockfd, buffer, sizeof(buffer)-1, 0);
            if (recv_bytes <= 0) {
                if (recv_bytes == 0) {
                    printf("[Thread %d] Server closed connection\n", i);
                } else {
                    perror("recv");
                }
                break;
            }

            buffer[recv_bytes] = '\0';
            printf("[Thread %d] Received: %s\n", i, buffer);

            // Optional: sleep to avoid flooding the server too fast
            usleep(1000); // 1ms
        } else {
            snprintf(message, sizeof(message), "get name%d\n", k);
            put = true;
            printf("[Thread %d] GET name%d\n", i, k);
            k++;  // Increment only after get
            count++;
            ssize_t sent_bytes = send(sockfd, message, strlen(message), 0);
            if (sent_bytes < 0) {
                perror("send");
                close(sockfd);
                pthread_exit(NULL);
            }

            char buffer[BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer));
            ssize_t recv_bytes = recv(sockfd, buffer, sizeof(buffer)-1, 0);
            ssize_t recv_bytes1 = recv(sockfd, buffer, sizeof(buffer)-1, 0);
            if (recv_bytes <= 0) {
                if (recv_bytes == 0) {
                    printf("[Thread %d] Server closed connection\n", i);
                } else {
                    perror("recv");
                }
                break;
            }

            buffer[recv_bytes] = '\0';
            printf("[Thread %d] Received: %s\n", i, buffer);

            // Optional: sleep to avoid flooding the server too fast
            usleep(1000); // 1ms
            }

        
    }

    close(sockfd);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[51];
    int num_threads = 50; // adjust as needed

    for (int i = 0; i < num_threads; i++) {
        int* arg = malloc(sizeof(*arg));
        *arg = i;
        if (pthread_create(&threads[i], NULL, send_and_receive, arg) != 0) {
            perror("pthread_create");
            free(arg);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Number of operations: %d\n", count);
    printf("All messages sent and responses received.\n");
    return 0;
}
