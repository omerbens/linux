

#ifndef _LINUX_PTREE_H
#define _LINUX_PTREE_H

typedef int (*ptree_func_t)(struct prinfo *buf, int *nr, int pid);
int register_ptree(ptree_func_t func);
void unregister_ptree(ptree_func_t func);

#endif /* _LINUX_PTREE_H */
