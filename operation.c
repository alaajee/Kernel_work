#include "operation.h"
#include "Client.h"
#include "treat.h"

// simple tache cpu pour commencer 


struct net_task {
    struct work_struct work;
    struct socket *client_sock;
};

void work_cpu(struct work_struct *cpu_work){
    // we execute it and then we add the work_net to the workqueue ( or do i initialize it , add the next_work to it?)
    struct cpu_task *task = container_of(cpu_work, struct cpu_task, work);

    int k = 0 ;
    int i;
    for (i = 0 ; i < MAX_LEN; i++ ){
        k += 1;
    }

    printk("je viens bien là");
    
    struct net_task *nt = kmalloc(sizeof(*nt), GFP_KERNEL);
    if (!nt) return;
    nt->client_sock = task->client_sock;
    INIT_WORK(&nt->work, net_cpu);
    queue_work(task_wq, &nt->work);
}

void net_cpu(struct work_struct *cpu_work){

    // envoyer une socket !!
    // struct client_work *cw = container_of(cpu_work, struct client_work, work_c);
   
   struct net_task *nt = container_of(cpu_work, struct net_task, work);
    if (!nt->client_sock) {
    printk(KERN_ERR "client_sock is NULL!\n");
    return;
}
    printk(KERN_INFO "nt->client_sock OK, on envoie le message\n");
    if (!nt->client_sock->sk) {
    printk(KERN_ERR "client_sock->sk NULL\n");
    return;
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
    

    INIT_WORK(&nt->work, client_handle);
    printk("eh bah wi wi ");
    queue_work(client_wq, &nt->work);

    printk(KERN_INFO "Réponse envoyée au client.\n");

}

EXPORT_SYMBOL(work_cpu);
EXPORT_SYMBOL(net_cpu);
MODULE_LICENSE("GPL");
