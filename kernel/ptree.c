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

int safe_ptree(struct prinfo *buf, int *nr, int pid) {
	int ret = -ENOSYS;
	spin_lock(&ptree_func_lock);
	if (NULL == ptree_func_ptr) {
		printk("do_ptree: no ptree func registerd\n");
		return ret;
	}
	ret = ptree_func_ptr(buf, nr, pid)
	spin_unlock(&ptree_func_lock);
	return ret;
}

// validate value for time getting
ptree_func get_safe_ptr() {
	ptree_func foo;
	spin_lock(&ptree_func_lock);
	foo = ptree_func_ptr(buf, nr, pid)
	spin_unlock(&ptree_func_lock);
	return foo;
}

int do_ptree(struct prinfo __user *buf, int __user *nr, int pid)
{
	int ret = -EFAULT;
	int k_nr;
	struct prinfo *k_buf = NULL;
	const char *my_module = "simple";

	if (NULL == buf || NULL == nr || 1 > *nr) {
		ret = -EINVAL;
		goto end;
	}

	if (0 == access_ok(nr, sizeof(int)) || 0 == access_ok(buf, sizeof())) {
		goto end;
	}

	printk("do_ptree: stated\n");

	if (NULL == get_safe_ptr()) {
		printk("do_ptree: trying to request module\n");
		request_module(my_module);
	}

	printk("do_ptree: copy from nr\n");
	if (copy_from_user(&k_nr, nr, sizeof(k_nr)))
		goto end;
	printk("do_ptree: copied from nr, value is %d\n", k_nr);

	printk("do_ptree: kmalloc buf\n");
	k_buf = kmalloc(k_nr * sizeof(*k_buf), GFP_KERNEL);
	if (NULL == k_buf)
		goto end_free;

	printk("do_ptree: calling func with nr of %d\n", k_nr);
	ret = safe_ptree(k_buf, &k_nr, pid);
	printk("do_ptree: calling func finished with nr of %d\n", k_nr);

	printk("do_ptree: copy to buf\n");
	if (copy_to_user(buf, k_buf, k_nr * sizeof(*k_buf)))
		goto end_free;

	printk("do_ptree: copy to nr\n");
	if (copy_to_user(nr, &k_nr, sizeof(k_nr)))
		goto end_free;

	printk("do_ptree: job done\n");
	goto end_free;

end_free:
	printk("do_ptree: free buf\n");
	kfree(k_buf);
end:
	printk("do_ptree: end\n");
	return ret;
}

EXPORT_SYMBOL(register_ptree);
EXPORT_SYMBOL(unregister_ptree);

SYSCALL_DEFINE3(ptree, struct prinfo __user *, buf, int __user *, nr, int, pid)
{
	return do_ptree(buf, nr, pid);
}
