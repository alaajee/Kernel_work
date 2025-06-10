#include "operation.h"
#include "Client.h"
#include "treat.h"

// simple tache cpu pour commencer 



void work_cpu(struct work_struct *cpu_work){
    // we execute it and then we add the work_net to the workqueue ( or do i initialize it , add the next_work to it?)
    
    struct connection_context *task = container_of(cpu_work, struct connection_context, cpu_task);

    int k = 0 ;
    int i;
    for (i = 0 ; i < MAX_LEN; i++ ){
        k += 1;
    }

    printk("je viens bien là");

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

    memset(&msg,0,sizeof(msg)); // nettoyer la structur
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
