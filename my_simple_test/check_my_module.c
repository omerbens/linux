
#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

#define PTEE_SYSCALL_NUM 449
#define MINIMUM_LINES 20

struct prinfo {
        pid_t parent_pid;       /* process id of parent */
        pid_t pid;              /* process id */
        long state;             /* current state of process */
        uid_t uid;              /* user id of process owner */
        char comm[16];          /* name of program executed */
        int level;              /* level of this process in the subtree */
};

int syscall_ptree(struct prinfo *buf, int *nr, int pid) {
	int ret;

//	printf("Calling system call for pid %d with nr of %d\n", pid, *nr);
        ret = syscall(PTEE_SYSCALL_NUM, buf, nr, pid);
//        printf("System call returned %d with nr of %d\n", ret, *nr);
        if (0 != ret){
                perror("Failed with");
		return -1;
	}
        return 0;
}

void print_prinfo(struct prinfo *buf, int size) {
	int i;
	printf("\nlevel     comm                pid       p_pid     state     uid\n");
	for (i=0; i<size; i++) {
		printf("%-10d%-20s%-10d%-10d%-10ld%-10d\n",
			buf[i].level,
			buf[i].comm,
			buf[i].pid,
			buf[i].parent_pid,
			buf[i].state,
			buf[i].uid);
	}
	printf("\n");
}

int print_all_ptree(int pid, int n) {
	struct prinfo *p;
	int original_n = n;
	int ret;

	p = calloc(n, sizeof(struct prinfo));
	if (NULL == p){
		printf("failed calloc");
		return -1;
	}

	ret = syscall_ptree(p,&n,pid);
	if (ret != 0)
		return ret;

	// syscall filled all space
	if (original_n == n) {
		free(p);
		return print_all_ptree(pid, n*2);
	}
	print_prinfo(p, n);
	free(p);
	return ret;
}

int main(int argc, char **argv)
{
        int pid;

	if (argc > 2) {
		printf("usage: check_my_module [pid]\n");
		return -1;
	}

	if (1 == argc)
		pid = 0;
	else
		pid = atoi(argv[1]);

	return print_all_ptree(pid, MINIMUM_LINES);
}
