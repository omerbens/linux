#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define SYSCALL_NUM 449
#define STARTING_LEN 500
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

int syscall_mapspages(unsigned long start, unsigned long end, char *buf, size_t size) {
	int ret;
	printf("	addresses values are %#lx and %#lx (length is: %#lx or %ld)\n", start, end, end-start, (end-start)/sysconf(_SC_PAGESIZE));
        ret = syscall(SYSCALL_NUM, start, end, buf, size);
        if (0 > ret){
                perror("Failed with");
		return -1;
	}
	printf("	return value is %d\n", ret);
        return ret;
}

void print_maps(unsigned long start, unsigned long end) {
	unsigned int buffer_length = STARTING_LEN;
	int ret;
	char *c;

	while (1 == 1) {
		c = malloc(buffer_length);
		ret = syscall_mapspages(start, end, c, buffer_length);
		if (ret == buffer_length) {
			free(c);
			buffer_length *= 2;
			continue;
		}
		printf("	output is:\n%s\n\n", c);
		free(c);
		return;
	}
	
}
int is_all_zero(int actions[], unsigned int len) {
	for (int i=0; i<len; i++)
		if (actions[i] != 0)
			return 0;
	return 1;
}

// ********************************************
// NOTE - Its doesnt work yet like expeceted.
// ********************************************
int action_on_pages(char *ptr, int actions[], unsigned int len) {
	printf("start action with len(%d), and arr: %d %d %d\n", len, actions[0],  actions[1],  actions[2]);

	if (1 == is_all_zero(actions, len)) {
		printf("done\n");
		return 1;
	}

	for (int i=0; i<len; i++) {
		if (actions[i] == 0){
			printf("	nothing\n");
			continue;
		}
		if (actions[i] == 1){
			printf("	locking (%d)\n", actions[i]);
			mlock(&ptr[PAGE_SIZE*i], PAGE_SIZE);
			actions[i] = 0;
			continue;
		}
		if (actions[i] == 2){
			printf("	writing (%d)\n", actions[i]);
			ptr[PAGE_SIZE*i] = 0;
			actions[i] = 0;
			continue;
		}
		if (actions[i] >= 2){
			printf("	writing (%d)\n", actions[i]);
			ptr[PAGE_SIZE*i] = 1;
			actions[i]=actions[i]-1;
			continue;
		}
	}

	if (1 == is_all_zero(actions, len)) {
		printf("done\n");
		return 1;
	}

	int pid = fork();
	if (-1 == pid) {
		printf("fork failed\n");
		return 0;
	}
	if (pid > 0)
	{
		// father - wait for child
		printf("	father - waits\n");
		while (wait(NULL) > 0);
		printf("	father - end\n");
		exit(0);
	}
	// child continue
	printf("	child - continuing\n");

	return action_on_pages(ptr, actions, len);
}



void generic(int testnum, char *s) {
	printf("start\n");
	int len = (int) strlen(s);
	char *ptr;

	ptr = mmap(0, PAGE_SIZE*len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (ptr == MAP_FAILED) {
		perror("failed mmap");
		return;
	}

	int *actions =  malloc(len * sizeof(int));
	if (NULL == malloc) {
		printf("failed malloc\n");
		return;
	}
	for (int i=0; i<len; i++) {
		if ('.' == s[i])
			actions[i] = 0;
		else if ('0' <= s[i] && '9' >= s[i])
			actions[i] = s[i]-'0';
		else if ('X' == s[i])
			actions[i] = 10;
		else {
			printf("unkown value: %c\n", s[i]);
			return;
		}
	}

	printf("calling action with: %d %d %d\n", actions[0],  actions[1],  actions[2]);
	int ret = action_on_pages(ptr, actions, len);
	if (1 != ret) {
		printf("failed to action_on_pages\n");
		return;
	}

	printf("\ntest%d \"%s\"\n", testnum, s);
	print_maps((unsigned long) ptr, (unsigned long)(ptr+PAGE_SIZE*len));

	if (-1 == munmap(ptr, PAGE_SIZE*len))
		printf("failed to munmap\n");
}

void print_stack(int fork_times) {
	unsigned long start, end;
	FILE *cmd = popen("cat /proc/$(pgrep check)/maps | grep \"\\[stack\\]\" | awk '{print $1}' | tr '-' '\n'", "r");
	if (NULL == cmd) {
		printf("failed popen");
		return;
	}
	if (!fscanf(cmd, "%lx%lx", &start, &end)){
		printf("failed scanf");
		return;
	}

	for (int i=0; i<fork_times; i++) {
		int pid = fork();
		if (-1 == pid) {
			return;
		}
		if (pid > 0)
		{
			// father - wait for child
			while (wait(NULL) > 0);
			exit(0);
		}
		// child continue
	}

	print_maps(start, end);
}


void main(int argc, char **argv) {
	char *c;
	char *tmp;

	if (argc > 3 || argc == 1 || strlen(argv[1]) != 1) {
		printf("usage: check <digit_test_num> [wanted_str_for_test8]\n");
		return;
	}

	switch (argv[1][0])
	{
		case '1':
			generic(1, "..........");
			break;
		case '2':		
			generic(2, "1111111111");
	    		break;
		case '3':
			generic(3, ".1.1.1.1.1");
			break;
		case '4':
			generic(4, "22222.....");
			break;
		case '5':
			generic(5, "1111..2222");
			break;
		case '6':
			tmp = malloc(2001);
			if (NULL == tmp) {
				printf("failed\n");
				return;
			}
			for (int i=0; i<2000; i++) {tmp[i]='.';}
			generic(6, tmp);
			free(tmp);
			break;
		case '7':
			// ********************************************
			// Todo stack
			// ********************************************
			print_stack(1000);
			sleep(20);
			break;
		case '8':
			if (3 == argc)
				generic(8, argv[2]);
			else
				printf("usage: check <digit_test_num> [wanted_str_for_test8]\n");
			break;
		case '9':
			print_stack(1000000);
			break;
		default:
			printf("unkown digit_test_num: %s\n", argv[1]);
			break;
	}
}

