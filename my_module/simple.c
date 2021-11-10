#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/prinfo.h>

typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);
extern int register_ptree(ptree_func func);
extern void unregister_ptree(ptree_func func);

MODULE_DESCRIPTION ("mine");
MODULE_LICENSE ("GPL");

int ptree_impl(struct prinfo *buf, int *nr, int pid) {
	return 0;
}

static int sample_init(void) {
	printk("module loaded\n");
	register_ptree(&ptree_impl);
	printk("register function\n");
	unregister_ptree(&ptree_impl);
	printk("unregister function\n");
	return 0;
}

static void sample_exit(void) {
        printk("module unloaded\n");
}

module_init(sample_init);
module_exit(sample_exit);
