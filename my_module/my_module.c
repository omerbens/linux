#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pagewalk.h>
#include <linux/sched/signal.h>

typedef int (*my_func_t)(unsigned long start, unsigned long end, char *buf, size_t size);
extern int register_func(my_func_t func);
extern void unregister_func(my_func_t func);

MODULE_DESCRIPTION ("mine");
MODULE_LICENSE ("GPL");

static int format_map(pmd_t *pmd, unsigned long addr, unsigned long end, struct mm_walk *walk) {
	struct vm_area_struct *vma = walk->vma;
	// char *buf = (char *) walk->private;
	// struct mm_struct *mm = vma->vm_mm;
	struct file *file = vma->vm_file;
	vm_flags_t flags = vma->vm_flags;
	unsigned long ino = 0;
	unsigned long long pgoff = 0;
	unsigned long start_addr, end_addr;
	dev_t dev = 0;

	
	if (file) {
		struct inode *inode = file_inode(vma->vm_file);
		dev = inode->i_sb->s_dev;
		ino = inode->i_ino;
		pgoff = ((loff_t)vma->vm_pgoff) << PAGE_SHIFT;
	}

	start_addr = vma->vm_start;
	end_addr = vma->vm_end;

	printk("%012lx-%012lx %c%c%c%c %#08llx %02x:%02x %-26ld <details>\n",
		start_addr, end_addr,
		flags & VM_READ ? 'r' : '-', flags & VM_WRITE ? 'w' : '-', flags & VM_EXEC ? 'x' : '-',  flags & VM_MAYSHARE ? 's' : 'p',
		pgoff, MAJOR(dev), MINOR(dev), ino);
	return 0;
}

static const struct mm_walk_ops find_maps = {
	.pmd_entry		= format_map,
};


int mapspages_impl(unsigned long start, unsigned long end, char *buf, size_t size) {
	printk("start\n");
   	walk_page_range(current->mm, start, end, &find_maps, buf);
	printk("end\n");
    return 1;
}

static int sample_init(void) {
	printk("module loaded: start\n");
	printk("register function: start\n");
	register_func(mapspages_impl);
	printk("register function: end\n");
	printk("module loaded: end\n");
	return 0;
}

static void sample_exit(void) {
	printk("module unloaded: start\n");
	printk("unregister function: start\n");
	unregister_func(mapspages_impl);
	printk("unregister function: end\n");
	printk("module unloaded: end\n");
}

module_init(sample_init);
module_exit(sample_exit);
