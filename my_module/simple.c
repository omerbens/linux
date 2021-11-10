#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/prinfo.h>

typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);
extern int register_ptree(ptree_func func);
extern void unregister_ptree(ptree_func func);

MODULE_DESCRIPTION ("mine");
MODULE_LICENSE ("GPL");

int getptree(struct prinfo *buf, int *nr, int pid) {
	const char *dummpy = "dummy";
	int i;

	if (NULL == buf || NULL == nr || 1 > *nr) {
		return -EINVAL;
	}
	for (i=0; i<*nr; i++) {
		buf[i].parent_pid=i;
		buf[i].pid=i+1;
		buf[i].state=0;
		buf[i].uid=0;
		strcpy(buf[i].comm, dummpy);
		buf[i].level=i;
	}
	return 0;
}

static int sample_init(void) {
	printk("module loaded: start\n");
	printk("register function: start\n");
	register_ptree(&getptree);
	printk("register function: end\n");
	printk("module loaded: end\n");
	return 0;
}

static void sample_exit(void) {
	printk("module unloaded: start\n");
	printk("unregister function: start\n");
	unregister_ptree(&getptree);
	printk("unregister function: end\n");
	printk("module unloaded: end\n");
}

module_init(sample_init);
module_exit(sample_exit);
