#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

// Module metadata
MODULE_AUTHOR("Santeri Röning");
MODULE_DESCRIPTION("My first kernel module");
MODULE_LICENSE("GPL");

static struct proc_dir_entry* proc_entry;

static ssize_t custom_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
	printk(KERN_INFO "My first kernel greetings!");

	char greeting[] = "Hello world!\n";
	int greeting_length = strlen(greeting); 
	
	// String is a array of chars
	if (*offset > 0)
		return 0; 

	copy_to_user(user_buffer, greeting, greeting_length);
	*offset = greeting_length; 
 
	return greeting_length;
}

// Creating the read -> greetings functionality
static struct proc_ops fops =
{
	.proc_read = custom_read
};

// Custom init and exit methods
static int __init custom_init(void) {
	proc_entry = proc_create("firstmodule", 0666, NULL, &fops);
	printk(KERN_INFO "My first kernel module loaded. \n");
	return 0;
}

static void __exit custom_exit(void) {
	printk(KERN_INFO "Bye. \n");
}

module_init(custom_init);
module_exit(custom_exit);
