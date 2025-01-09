// hello_world.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

// Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World Kernel Module");

// Init function
static int __init hello_world_init(void) {
    
    printk(KERN_INFO "Hello, world! The module has been loaded.\n");
    return 0; // Return 0 to indicate success
}

// Exit function
static void __exit hello_world_exit(void) {
    printk(KERN_INFO "Goodbye, world! The module is being unloaded.\n");
}

// Register the init and exit functions
module_init(hello_world_init);
module_exit(hello_world_exit);
