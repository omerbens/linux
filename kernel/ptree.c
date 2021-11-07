#include <linux/kernel.h>
#include <linux/syscalls.h>

typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);

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

SYSCALL_DEFINE3(getptree, struct prinfo *, buf, int *, nr, int, pid)
{
        return do_getptree(buf, nr, pid);
}
