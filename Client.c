#include "Client.h"
#include "operation.h"
#include "SocketHandler.h"
// struct client_work *c_work;

void client_handle(struct work_struct *work)
{
    // here i can create myBD
    // I can after that add this db to my structure cw
    
    struct connection_context *cw = container_of(work, struct connection_context, work_c);
    
    // uint8_t *buf;
    // int ret;
    
    // if (!cw->client_sock){
    // 	printk("eeeeeeeeeeeeeeeeeeeeeeehhhhhhhhhh");
	//     goto clean;
    // }
    // buf = kmalloc(BUF_SIZE, GFP_KERNEL);
    // if (!buf) {
    //     pr_err("%s: Failed to allocate memory for buffer\n", THIS_MODULE->name);
    //     goto clean;
    // }
    // char *data  = buf;
    // struct kvec iov = {
    //     .iov_base = buf,
    //     .iov_len = BUF_SIZE,
    // };

    // struct msghdr msg = {0};

    // ret = kernel_recvmsg(cw->client_sock, &msg, &iov, 1, BUF_SIZE, 0); // on lit du msg et on stocke dans iov
    
    // if (ret < 0) {
    //     pr_err("%s: kernel_recvmsg failed: %d\n", THIS_MODULE->name, ret);
    //    	kernel_sock_shutdown(cw->client_sock, SHUT_RDWR);
    //     sock_release(cw->client_sock);
    //     kfree(buf);
    //    	goto clean;
    // }
    // // here if i have the message received is put i call work_store 

    KrDb* db = NULL; /* used differently by all switch cases */

    KrOutbuf outbuf = kr_outbuf(sizeof(u64));

    cw->outbuf = &outbuf; // on stocke l'outbuf dans le cw

    /* helper macros for reading message data */
    // here we extract the NEXT_U8, NEXT_U64, NEXT_PTR macros
    #define NEXT_U8()  (*(u8* )((data += sizeof(u8))  - sizeof(u8)))
    // #define NEXT_U64() (*(u64*)((data += sizeof(u64)) - sizeof(u64)))
    // #define NEXT_PTR(len) ((data += len) - len)
    #define GET_DB() do {                                       \
            db = kr_db_from_id(NEXT_U8());                      \
            if (!db) {                                          \
                printk(KERN_INFO "GET_DB WITH INVALID ID\n");   \
                goto clean;                                         \
            }                                                   \
        } while (0)
    
    int ret = kr_db_open(&db, "/dev/loop18");

    if (ret < 0) {
        pr_err("%s: kr_db_open failed: %d\n", THIS_MODULE->name, ret);
        goto clean;
    }
    cw->db = db; // on stocke la db dans le cw
    // // here we will replace the work_cpu with a key/value task 
    begin_work(work);
goto end;

clean:
    // we have to free the cw 
    kfree(cw);
    pr_err("%s: Error in client_handle, freeing connection context\n", THIS_MODULE->name);
    kernel_sock_shutdown(cw->client_sock, SHUT_RDWR);
    sock_release(cw->client_sock);
    return;
end:
    return;
}

void begin_work(struct work_struct *work)
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
    char *data  = buf;
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
    cw->data = data; // on stocke le data dans le cw
    if (strncmp(data,"put" , 3) == 0) {
        INIT_WORK(&cw->store_task, work_store);
        queue_work(client_wq, &cw->store_task);
    } else if (strncmp(data, "get", 3) == 0) {      
        INIT_WORK(&cw->get_task, work_get);
        queue_work(client_wq, &cw->get_task);
    } else {
        pr_err("%s: Unknown command received: %s\n", THIS_MODULE->name, data);
        goto clean;
    }
goto end;

clean:
    // we have to free the cw 
    kfree(cw);
    pr_err("%s: Error in client_handle, freeing connection context\n", THIS_MODULE->name);
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