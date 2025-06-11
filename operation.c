#include "operation.h"
#include "Client.h"
#include "treat.h"

// simple tache cpu pour commencer 

#define SIZE 100  // Adjust size as needed

void work_cpu(struct work_struct *cpu_work){
    struct connection_context *task = container_of(cpu_work, struct connection_context, cpu_task);
     
    // better to allocate memory ?? 
    int matrix [SIZE][SIZE];
    int matrix2 [SIZE][SIZE];
    int k = 0;

    for (int i = 0; i < 15; i++) {
        for (int  j = 0; j < 15; j++){
            matrix2[i][j] = i + j; 
            matrix[i][j] = matrix2[i][j] * matrix2[i][j]; 
            k += matrix[i][j]; 
         }
    }

    printk(KERN_INFO "CPU work done, k = %d\n", k);
    // Initialize and queue the network work
    INIT_WORK(&task->net_task, net_cpu);
    queue_work(task_wq, &task->net_task);
}
void net_cpu(struct work_struct *cpu_work){

    // envoyer une socket !!
    // struct client_work *cw = container_of(cpu_work, struct client_work, work_c);
   
    struct connection_context *nt = container_of(cpu_work, struct connection_context, net_task);
    nt->mySocket++;
    if (!nt->client_sock) {
        printk(KERN_ERR "client_sock is NULL!\n");
        goto clean;
    }
    printk(KERN_INFO "nt->client_sock OK, on envoie le message\n");
    if (!nt->client_sock->sk) {
        printk(KERN_ERR "client_sock->sk NULL\n");
        goto clean;
    }
    char *data = "ok";
    
    struct msghdr msg;
    struct kvec vec;

    vec.iov_base = data;
    vec.iov_len = strlen(data);

    memset(&msg,0,sizeof(msg)); // nettoyer la structure
    int ret = kernel_sendmsg(nt->client_sock, &msg, &vec,1,vec.iov_len); // envoyer le message dans le kernel 
    printk("je finis mon send");
    if (ret < 0) {
        printk(KERN_ERR "Erreur d'envoi au client : %d\n", ret);
    }
    printk(KERN_INFO "%d\n" , nt->mySocket);
clean:
    if (nt->mySocket == 3){
        kernel_sock_shutdown(nt->client_sock, SHUT_RDWR);
        sock_release(nt->client_sock);
        return;
    }
     
  

INIT_WORK(&nt->work_c, client_handle);
queue_work(client_wq, &nt->work_c);

    

}
EXPORT_SYMBOL(work_cpu);
EXPORT_SYMBOL(net_cpu);
MODULE_LICENSE("GPL");
