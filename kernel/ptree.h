

#ifndef _LINUX_PTREE_H
#define _LINUX_PTREE_H

typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);
int register_ptree(ptree_func func);
void unregister_ptree(ptree_func func);

#endif /* _LINUX_PTREE_H */
