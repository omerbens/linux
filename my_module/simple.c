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

int getptree(struct prinfo *_buf, int *nr, int pid) {
	const char *dummpy = "dummy";
	int i;

	if (NULL == _buf || NULL == nr || 1 > *nr) {
		return -EINVAL;
	}

	for (i=0; i<*nr; i++) {
		struct prinfo *_new = kmalloc(sizeof(struct prinfo), GFP_KERNEL);
		_new->parent_pid=i;
		_new->pid=i+1;
		_new->state=0;
		_new->uid=0;
		strcpy(_new->comm, dummpy);
		_new->level=i;
	}
	return 0;
}

static int sample_init(void) {
	printk("module loaded\n");
	register_ptree(&getptree);
	printk("register function\n");
	unregister_ptree(&getptree);
	printk("unregister function\n");
	return 0;
}

static void sample_exit(void) {
        printk("module unloaded\n");
}

module_init(sample_init);
module_exit(sample_exit);
