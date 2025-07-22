#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>

#define BUFFER_SIZE 512

// Thread-safe counter
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
int count = 0;

void safe_increment_count() {
    pthread_mutex_lock(&count_mutex);
    count++;
    pthread_mutex_unlock(&count_mutex);
}

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
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        pthread_exit(NULL);
    }
    
    // Small delay after connection to let server initialize
    usleep(10000); // 10ms
    
    int k = 0;
    bool put = true;
    char message[128];
    
    for (int j = 0; j < 5; j++) {
        if (put) {
            snprintf(message, sizeof(message), "put name%d alaa%d\n", k, k);
            put = false;
            printf("[Thread %d] PUT name%d -> alaa%d\n", i, k, k);
            
            safe_increment_count();
            
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
            
            // Delay between operations to avoid overwhelming server
            usleep(50000); // 50ms
            
        } else {
            snprintf(message, sizeof(message), "get name%d\n", k);
            put = true;
            printf("[Thread %d] GET name%d\n", i, k);
            k++; // Increment only after get
            
            safe_increment_count();
            
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
            
            // Your original code had a second recv for GET - keep it if needed
            // Uncomment the next lines if your server requires it:
            /*
            ssize_t recv_bytes2 = recv(sockfd, buffer, sizeof(buffer)-1, 0);
            if (recv_bytes2 > 0) {
                buffer[recv_bytes2] = '\0';
                printf("[Thread %d] Received (2nd): %s\n", i, buffer);
            }
            */
            
            // Delay between operations to avoid overwhelming server
            usleep(50000); // 50ms
        }
    }
    
    close(sockfd);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[201];
    int num_threads = 200; // Test with 5 clients
    
    printf("Starting %d client threads...\n", num_threads);
    
    for (int i = 0; i < num_threads; i++) {
        int* arg = malloc(sizeof(*arg));
        if (arg == NULL) {
            perror("malloc");
            return EXIT_FAILURE;
        }
        
        *arg = i;
        if (pthread_create(&threads[i], NULL, send_and_receive, arg) != 0) {
            perror("pthread_create");
            free(arg);
            return EXIT_FAILURE;
        }
        
        // Stagger thread creation to avoid overwhelming server
        usleep(100000); // 100ms between thread creations
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&count_mutex);
    
    printf("Number of operations: %d\n", count);
    printf("All messages sent and responses received.\n");
    return 0;
}