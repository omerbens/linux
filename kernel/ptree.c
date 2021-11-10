#include <linux/kernel.h>
#include <linux/syscalls.h>
#include "ptree.h"

int register_ptree(ptree_func func){
	return 0;
}

void unregister_ptree(ptree_func func){
	return;
}

int do_getptree(struct prinfo *buf, int *nr, int pid)
{
        printk("Hello world\n");
        return 0;
}

EXPORT_SYMBOL(register_ptree);
EXPORT_SYMBOL(unregister_ptree);

SYSCALL_DEFINE3(ptree, struct prinfo *, buf, int *, nr, int, pid)
{
        return do_getptree(buf, nr, pid);
}
