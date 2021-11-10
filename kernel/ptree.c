#include <linux/kernel.h>
#include <linux/syscalls.h>
#include "ptree.h"

static DEFINE_SPINLOCK(ptree_func_lock);
static ptree_func ptree_func_ptr = NULL;

int register_ptree(ptree_func func)
{
	int ret = -EBUSY;

	spin_lock(&ptree_func_lock);
	if (NULL != ptree_func_ptr)
		goto end;
	ptree_func_ptr = func;
	ret = 0;
end:
	spin_unlock(&ptree_func_lock);
	return ret;
}

void unregister_ptree(ptree_func func)
{
	spin_lock(&ptree_func_lock);
	ptree_func_ptr = NULL;
	spin_unlock(&ptree_func_lock);
	return;
}

int do_ptree(struct prinfo __user *buf, int __user *nr, int pid)
{
	int ret = 0;
	struct prinfo *k_buf = NULL;
	int *k_nr = NULL;
	const char *my_module = "simple";

	printk("do_ptree stated\n");
	spin_lock(&ptree_func_lock);

	if (NULL != ptree_func_ptr) {
		goto func_registerd;
	}

	spin_unlock(&ptree_func_lock);
	printk("trying to request module\n");
	request_module(my_module);
	spin_lock(&ptree_func_lock);

	if (NULL == ptree_func_ptr) {
		printk("no ptree func registerd\n");
		ret = -ENOSYS;
		goto end;
	}

func_registerd:
	if (copy_from_user(k_nr, nr, sizeof(k_nr)))
		goto end_fault;
	if (copy_from_user(k_buf, buf, (*k_nr) * sizeof(k_buf)))
		goto end_fault;

	printk("do_ptree: calling func\n");
	ret = ptree_func_ptr(k_buf, k_nr, pid);
	printk("do_ptree: calling func finished\n");

	if (copy_to_user(buf, k_buf, (*k_nr) * sizeof(k_buf)))
		goto end_fault;
	if (copy_to_user(nr, k_nr, sizeof(k_nr)))
		goto end_fault;

	printk("do_ptree: job done\n");
	goto end;

end_fault:
	ret = -EFAULT;
end:
	spin_unlock(&ptree_func_lock);
	printk("do_ptree: end\n");
	return ret;
}

EXPORT_SYMBOL(register_ptree);
EXPORT_SYMBOL(unregister_ptree);

SYSCALL_DEFINE3(ptree, struct prinfo __user *, buf, int __user *, nr, int, pid)
{
	return do_ptree(buf, nr, pid);
}
