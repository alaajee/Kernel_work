#include <linux/module.h> 
#include <linux/printk.h> // for the macro expansion 
#include <linux/init.h> /* Needed for the macros ( module_init and module_exit) */
#include <linux/kernel.h>

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Jenninea"); 
MODULE_DESCRIPTION("Simple traitement des socket");

/*
    [ Réception réseau (client) ]
               │
               ▼
        [ Traitement CPU ]
               │
               ▼
   [ Réponse réseau (client) ]
*/

