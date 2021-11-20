#include <linux/kernel.h>
#include <linux/syscalls.h>
#include "mysyscall.h"

static DEFINE_SPINLOCK(func_lock);
static my_func_t my_func_ptr = NULL;

int register_func(my_func_t func)
{
	int ret = -EBUSY;

	spin_lock(&func_lock);
	if (NULL != my_func_ptr)
		goto end;
	my_func_ptr = func;
	ret = 0;
end:
	spin_unlock(&func_lock);
	return ret;
}

void unregister_func(my_func_t func)
{
	spin_lock(&func_lock);
	my_func_ptr = NULL;
	spin_unlock(&func_lock);
	return;
}

int safe_mapspages(unsigned long start, unsigned long end, char *buf, size_t size) {
	int ret = -ENOSYS;

	spin_lock(&func_lock);
	if (NULL == func_ptr)
		goto end;
	ret = func_ptr(start, end, buf, size);
end:
	spin_unlock(&func_lock);
	return ret;
}

int do_mapspages(unsigned long start, unsigned long end, char __user *buf, size_t size)
{
	int ret = -EFAULT;
	char * kbuf;

	printk("do_mapspages: stated\n");

	if (NULL == buf || start > end) {
		ret = -EINVAL;
		goto end;
	}

	if (!access_ok(buf, size))
		goto end;

	printk("do_mapspages: kmalloc buf\n");
	k_buf = kmalloc(size, GFP_KERNEL);
	if (NULL == k_buf)
		goto end;

	printk("do_mapspages: calling func with size of %d\n", size);
	ret = safe_ptree(start, end, k_buf, size);
	printk("do_mapspages: calling func finished with ret of %d\n", ret);

	printk("do_mapspages: copy to buf\n");
	if (copy_to_user(buf, k_buf, ret))
		goto end_free;

end_free:
	printk("do_mapspages: free buf\n");
	kfree(k_buf);
end:
	printk("do_mapspages: end\n");
	return ret;
}

EXPORT_SYMBOL(register_func);
EXPORT_SYMBOL(unregister_func);

SYSCALL_DEFINE4(mapspages, unsigned long, start, unsigned long, end, char __user, *buf, size_t, size)
{
//	return do_ptree(buf, nr, pid);
	return 0;
}
