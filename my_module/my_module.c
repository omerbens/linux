#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pagewalk.h>
#include <linux/sched/signal.h>

typedef int (*my_func_t)(unsigned long start, unsigned long end, char *buf, size_t size);
extern int register_func(my_func_t func);
extern void unregister_func(my_func_t func);
extern int walk_page_range(struct mm_struct *mm, unsigned long start, unsigned long end, const struct mm_walk_ops *ops, void *private);

MODULE_DESCRIPTION ("mine");
MODULE_LICENSE ("GPL");

// 0 - succeeded to handle the current entry, and if you don't reach the end address yet, continue to walk
// >0 : succeeded to handle the current entry, and return to the caller with caller specific value.
// <0 : failed to handle the current entry, and return to the caller with error code.
static int format_map(pmd_t *pmd, unsigned long addr, unsigned long end, struct mm_walk *walk) {
	char *comm = (char *) walk->private;
	// struct vm_area_struct *vma = walk->vma;
	printk("%s\n", comm);
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
