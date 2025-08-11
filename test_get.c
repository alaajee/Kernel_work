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
#define NUM_OPERATIONS 5  // Fixed to match your actual loop

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
    uint64_t start_time, end_time, latency_time;
    
    printf("[Thread %d] Starting operations on socket %d\n", i, sockfd);
    
    // Phase GET
    for (k = 0; k < 5; k++) {
        memset(buffer, 0, sizeof(buffer));
        snprintf(message, sizeof(message), "get name%d\n", k);
        
        printf("[Thread %d] Sending: %s", i, message);
        start_time = now_us();
        
        // Send message
        ssize_t sent = send(sockfd, message, strlen(message), 0);
        if (sent < 0) {
            perror("send failed");
            close(sockfd);
            return NULL;
        }
        
        // Receive response
        int total = 0;
        bool response_complete = false;
        
        while (!response_complete && total < sizeof(buffer) - 1) {
            ssize_t recv_bytes = recv(sockfd, buffer + total, sizeof(buffer) - total - 1, 0);
            
            if (recv_bytes < 0) {
                perror("recv failed");
                close(sockfd);
                return NULL;
            } else if (recv_bytes == 0) {
                printf("[Thread %d] Connection closed by server\n", i);
                close(sockfd);
                return NULL;
            }
            
            total += recv_bytes;
            buffer[total] = '\0';
            
            // Check if we have a complete response (ends with newline)
            if (strchr(buffer, '\n') != NULL) {
                response_complete = true;
            }
        }
        
        end_time = now_us();
        latency_time = end_time - start_time;
        
        // Clean up the response (remove trailing newline for display)
        char* newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        
        printf("[Thread %d] Received: '%s' | Latency: %lu us\n", i, buffer, latency_time);
        
        // Small delay between operations to avoid overwhelming the server
        // usleep(1000); // 1ms delay
    }
    
    printf("[Thread %d] Closing socket %d\n", i, sockfd);
    close(sockfd);
    return NULL;
}

int main() {
    pthread_t threads[101];
    int num_threads = 1;
    uint64_t start_time = now_us();
    
    printf("Starting %d client threads for GET operations...\n", num_threads);
    
    for (int i = 0; i < num_threads; i++) {
        // Create socket
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket failed");
            continue;
        }
        
        // Configure server address
        struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(12345),
            .sin_addr.s_addr = htonl(INADDR_LOOPBACK)
        };
        
        // Connect to server
        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("connect failed");
            close(sockfd);
            continue;
        }
        
        printf("Thread %d connected on socket %d\n", i, sockfd);
        
        // Prepare thread arguments
        thread_args* args = malloc(sizeof(thread_args));
        if (!args) {
            perror("malloc failed");
            close(sockfd);
            continue;
        }
        
        args->thread_id = i;
        args->sockfd = sockfd;
        
        // Create thread
        if (pthread_create(&threads[i], NULL, client_operations, args) != 0) {
            perror("pthread_create failed");
            free(args);
            close(sockfd);
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    uint64_t end_time = now_us();
    double elapsed_sec = (end_time - start_time) / 1e6;
    int total_operations = num_threads * NUM_OPERATIONS;
    double throughput = total_operations / elapsed_sec;
    
    printf("All GET operations completed.\n");
    printf("Elapsed time: %.3f seconds\n", elapsed_sec);
    printf("Total operations: %d\n", total_operations);
    printf("Throughput: %.2f operations per second\n", throughput);
    
    return 0;
}