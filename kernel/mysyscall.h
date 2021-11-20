#ifndef _LINUX_MYSYSCALL_H
#define _LINUX_MYSYSCALL_H

typedef int (*my_func_t)(unsigned long start, unsigned long end, char __user *buf, size_t size);
int register_func(my_func_t func);
void unregister_func(my_func_t func);

#endif /* _LINUX_MYSYSCALL_H */
