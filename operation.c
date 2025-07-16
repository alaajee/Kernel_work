#include "operation.h"
#include "Client.h"


// simple tache cpu pour commencer 

#define SIZE 100  // Adjust size as needed

// for the store and the get , we can use just one database that i will initialize in main.c and here i manipulate the store etc
void work_store(struct work_struct *store_work){
    struct connection_context *cn = container_of(store_work, struct connection_context, store_task);
    char* data= cn->data; // on recupere le data du 
    char key[256];

    if (sscanf(data, "put %s", key) == 1) {
        printk("Key: %s\n", key);
    }

    char value[256];
    if (sscanf(data, "put %*s %s", value) == 1) {
        printk("Value: %s\n", value);
    } 
    
    int ret = kr_db_put(cn->db, (KrSlice){.data = key , .size = strlen(key)}, (KrSlice){.data = value, .size = strlen(value)});
    if (ret < 0) {
        printk(KERN_ERR "Error in kr_db_put: %d\n", ret);
        return;
    }
    printk(KERN_INFO "Put operation successful\n");
    // Initialize and queue the network work
    INIT_WORK(&cn->cpu_task, work_cpu);
    queue_work(task_wq, &cn->cpu_task);
}

void work_get(struct work_struct *get_work){
    struct connection_context *cn = container_of(get_work, struct connection_context, get_task);
    // Here we can implement the get operation, for now we just print a message
    u64 size;
    char *key = kmalloc(256, GFP_KERNEL);
    if (!key) return;

    if (sscanf(cn->data, "get %s", key) == 1) {
        printk(KERN_INFO "Key to get: %s\n", key);
    } else {
        printk(KERN_ERR "Failed to parse key\n");
        kfree(key);
        return;
    }


    int ret = kr_db_get(cn->db, (KrSlice){.data = key , .size =  strlen(key)}, cn->outbuf,&size,cn);
    if (ret < 0) {
        printk(KERN_ERR "Error in kr_db_get: %d\n", ret);
        return;
    }
    printk(KERN_INFO "Get operation successful\n");
    INIT_WORK(&cn->cpu_task, work_cpu);
    queue_work(task_wq, &cn->cpu_task);
}
void work_cpu(struct work_struct *cpu_work){
    struct connection_context *cn = container_of(cpu_work, struct connection_context, cpu_task);
     
    // better to allocate memory ?? 
    int matrix [SIZE][SIZE];
    int matrix2 [SIZE][SIZE];
    int k = 0;

    for (int i = 0; i < SIZE; i++) {
        for (int  j = 0; j < SIZE; j++){
            matrix2[i][j] = i + j; 
            matrix[i][j] = matrix2[i][j] * matrix2[i][j]; 
            k += matrix[i][j]; 
         }
    }

    printk(KERN_INFO "CPU work done, k = %d\n", k);
    // Initialize and queue the network work
    INIT_WORK(&cn->net_task, work_net);
    queue_work(task_wq, &cn->net_task);
}

void work_net(struct work_struct *net_work){

    // envoyer une socket !!
    // struct client_work *cw = container_of(cpu_work, struct client_work, work_c);
   
    struct connection_context *cn = container_of(net_work, struct connection_context, net_task);
    cn->mySocket++;
    if (!cn->client_sock) {
        printk(KERN_ERR "client_sock is NULL!\n");
        goto clean;
    }
    printk(KERN_INFO "nt->client_sock OK, on envoie le message\n");
    if (!cn->client_sock->sk) {
        printk(KERN_ERR "client_sock->sk NULL\n");
        goto clean;
    }
    char *data = "ok";
    
    struct msghdr msg;
    struct kvec vec;

    vec.iov_base = data;
    vec.iov_len = strlen(data);

    memset(&msg,0,sizeof(msg)); // nettoyer la structure
    int ret = kernel_sendmsg(cn->client_sock, &msg, &vec,1,vec.iov_len); // envoyer le message dans le kernel 
    printk("je finis mon send");
    if (ret < 0) {
        printk(KERN_ERR "Erreur d'envoi au client : %d\n", ret);
        kfree(cn);
        kernel_sock_shutdown(cn->client_sock, SHUT_RDWR);
        sock_release(cn->client_sock);
    }
    printk(KERN_INFO "%d\n" , cn->mySocket);
clean:
    // This should be taken as an argument for the module 
    if (cn->mySocket == n){
        kfree(cn);
        kernel_sock_shutdown(cn->client_sock, SHUT_RDWR);
        sock_release(cn->client_sock);
        return;
    }
     
  

INIT_WORK(&cn->work_c, begin_work);
queue_work(client_wq, &cn->work_c);
}

EXPORT_SYMBOL(work_cpu);
EXPORT_SYMBOL(work_net);
MODULE_LICENSE("GPL");
