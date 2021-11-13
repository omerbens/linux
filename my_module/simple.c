#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/prinfo.h>
#include <linux/sched.h>
#include <linux/init_task.h>

typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);
extern int register_ptree(ptree_func func);
extern void unregister_ptree(ptree_func func);

struct task_level {
	int level;
	struct task_struct *value;
};

MODULE_DESCRIPTION ("mine");
MODULE_LICENSE ("GPL");

void write_data(struct prinfo *buf, struct task_struct *t, int level) {
	buf->pid = t->tgid;
	buf->parent_pid = t->parent->tgid;
	buf->state = t->state;
	buf->uid=t->cred->uid.val;
	strncpy(buf->comm, t->comm, 15);
	buf->level=level;
}

int getptree(struct prinfo *buf, int *nr, int pid) {
	int current_level, is_added, count, i;
	struct task_level *bfs;
	struct task_struct *t, *s;

	if (NULL == buf || NULL == nr || 1 > *nr || pid < 0)
		return -EINVAL;

	bfs = kmalloc((*nr) * sizeof(struct task_level), GFP_KERNEL);
	if (NULL == bfs)
		return -EFAULT;

	rcu_read_lock();
	if (0 == pid)
		t = &init_task;
	else
		t = pid_task(find_vpid(pid), PIDTYPE_PID);
	if (NULL == t) {
		rcu_read_unlock();
		return -EINVAL;
	}
	bfs[0].level = 0;
	bfs[0].value = t;
	count = 1;
	current_level = 0;

	// filling bfs
	while (count < *nr) {
		is_added = 0;
		for (i=0; i<count && count < *nr; i++) {
			if (bfs[i].level != current_level)
				continue;

			list_for_each_entry(s, &bfs[i].value->children, sibling) {
				if (s->tgid != s->pid)
					continue;
				bfs[count].level = current_level+1;
				bfs[count].value = s;
				count++;
				is_added = 1;

				if (!(count < *nr))
					break;
			}
		}
		current_level++;

		// no new children
		if (0 == is_added)
			break;
	}

	// update nr
	*nr = count;

	// writing bfs to buf
	for (i=0; i<count; i++) {
		write_data(&buf[i], bfs[i].value, bfs[i].level);
	}
	rcu_read_unlock();

	kfree(bfs);
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
