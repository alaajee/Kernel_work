#include "Client.h"
#include "treat.h"


struct client_work *c_work;

void client_handle(struct work_struct *work)
{
    struct connection_context *cw = container_of(work, struct connection_context, work_c);
    uint8_t *buf;
    int ret;
    
    if (!cw->client_sock){
    	printk("eeeeeeeeeeeeeeeeeeeeeeehhhhhhhhhh");
	    goto clean;
    }
    buf = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!buf) {
        pr_err("%s: Failed to allocate memory for buffer\n", THIS_MODULE->name);
        goto clean;
    }

    struct kvec iov = {
        .iov_base = buf,
        .iov_len = BUF_SIZE,
    };

    struct msghdr msg = {0};

    ret = kernel_recvmsg(cw->client_sock, &msg, &iov, 1, BUF_SIZE, 0); // on lit du msg et on stocke dans iov
    
    if (ret < 0) {
        pr_err("%s: kernel_recvmsg failed: %d\n", THIS_MODULE->name, ret);
       	kernel_sock_shutdown(cw->client_sock, SHUT_RDWR);
        sock_release(cw->client_sock);
        kfree(buf);
       	goto clean;
    }
 

    INIT_WORK(&cw->cpu_task, work_cpu);
    queue_work(task_wq, &cw->cpu_task);

goto end;

clean:
    kernel_sock_shutdown(cw->client_sock, SHUT_RDWR);
    sock_release(cw->client_sock);
end:
    if (buf)
      	kfree(buf);
}

// #include <linux/string.h>
// #include <linux/slab.h>
// #include <linux/uio.h>

// void client_handle(struct work_struct *work)
// {
//     struct client_work *cw = container_of(work, struct client_work, work_c);

//     char *msg = "OK from kernel\n";
//     struct msghdr msg_hdr = {0};
//     struct kvec vec;

//     vec.iov_base = msg;
//     vec.iov_len = strlen(msg);

//     kernel_sendmsg(cw->client_sock, &msg_hdr, &vec, 1, vec.iov_len);

//     sock_release(cw->client_sock);
//     kfree(cw);

//     printk(KERN_INFO "Réponse envoyée au client.\n");
// }



EXPORT_SYMBOL(client_handle);
MODULE_LICENSE("GPL");
