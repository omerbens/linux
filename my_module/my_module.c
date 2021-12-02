#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/pagewalk.h>
#include <linux/sched/signal.h>

typedef int (*my_func_t)(unsigned long start, unsigned long end, char *buf, size_t size);
extern int register_func(my_func_t func);
extern void unregister_func(my_func_t func);

MODULE_DESCRIPTION ("mine");
MODULE_LICENSE ("GPL");

struct writer {
	char *buf;
	unsigned int index;
	size_t size;
};

static int format_page(pte_t *pte, unsigned long addr, unsigned long next, struct mm_walk *walk){
	int written, refcount;
	struct page *page;
	struct vm_area_struct *vma = walk->vma;
	struct writer *w = (struct writer *) walk->private;
	char c = 'E';
	if (!pte_present(*pte)) {
		c = '.';
		goto end;
	}

	page = vm_normal_page(vma, addr, *pte);
	if (!page)
		goto end;

	refcount = page_ref_count(page);
	if (refcount >= 0 && refcount <= 9)
		c = '0' + refcount;
	else if (refcount > 9)
		c = 'X';

end:
	written = sprintf(&(w->buf[w->index]),"%c", c);
	if (written > 0)
		w->index += written;
	return 0;
}

static const struct mm_walk_ops single_page_ops = {
	.pte_entry		= format_page,
};

static int format_map(pmd_t *pmd, unsigned long addr, unsigned long end, struct mm_walk *walk) {
	struct vm_area_struct *vma = walk->vma;
	struct writer *w = (struct writer *) walk->private;
	struct file *file = vma->vm_file;
	vm_flags_t flags = vma->vm_flags;
	unsigned long ino = 0;
	unsigned long long pgoff = 0;
	unsigned long start_addr, end_addr;
	dev_t dev = 0;
	int written = 0;

	if (file) {
		struct inode *inode = file_inode(vma->vm_file);
		dev = inode->i_sb->s_dev;
		ino = inode->i_ino;
		pgoff = ((loff_t)vma->vm_pgoff) << PAGE_SHIFT;
	}

	start_addr = vma->vm_start;
	end_addr = vma->vm_end;


	// checking enough space
	// single line is 73 (with padding) + 1 for end of line (\n)
	// every page is 1 char, number of pages is {(end_addr-start_addr) / PAGE_SIZE}
	if (w->size - w->index < (end_addr-start_addr)/4096 + 74) {
		// doing a trick to inform that no space left
		// padding extra data with zeros
		sprintf(&(w->buf[w->index]),"%0*u", (int) w->size - w->index, 0);
		w->index = w->size;

		// negative value means to abort the current walk
		return -1;
	}


	written = sprintf(&(w->buf[w->index]),"%012lx-%012lx %c%c%c%c %#08llx %02x:%02x %-26ld ",
		start_addr, end_addr,
		flags & VM_READ ? 'r' : '-', flags & VM_WRITE ? 'w' : '-', flags & VM_EXEC ? 'x' : '-',  flags & VM_MAYSHARE ? 's' : 'p',
		pgoff, MAJOR(dev), MINOR(dev), ino);
	if (written > 0)
		w->index += written;

	walk_page_vma(vma, &single_page_ops, w);

	written = sprintf(&(w->buf[w->index]),"\n");
	if (written > 0)
		w->index += written;
	return 0;
}

static const struct mm_walk_ops page_range_ops = {
	.pmd_entry		= format_map,
};


int mapspages_impl(unsigned long start, unsigned long end, char *buf, size_t size) {
	struct writer w;
	printk("start\n");

	w.buf = buf;
	w.index = 0;
	w.size = size;

	if (1 != down_read_trylock(&current->mm->mmap_sem))
		return -1;
	walk_page_range(current->mm, start, end, &page_range_ops, &w);
	up_read(&current->mm->mmap_sem);

	printk("end\n");
    return w.index;
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
