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

void generic(int testnum,char *s) {
	unsigned long addr;
	long PAGE_SIZE = sysconf(_SC_PAGESIZE);
	int pid, wpid, status;
	int len = (int) strlen(s);
	char *ptr, *ptr2;

	addr = (unsigned long) mmap(0, PAGE_SIZE*len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if ((void*)addr == MAP_FAILED) {
		perror("failed mmap");
		return;
	}

	ptr = (char*) addr;
	for (int i=0; i<len; i++) {
		switch (s[i])
		{
			case '.':
				break;
			case '1':
				ptr[PAGE_SIZE*i] = '0';
		    		break;
			case '2':
				break;
			default:
				printf("unkown value: %c\n", s[i]);
				return;
		}
	}

	printf("test%d \"%s\"\n", testnum, s);
	print_maps(addr, addr+PAGE_SIZE*len);
	munmap((void*)addr, PAGE_SIZE*len);
}

void main(int argc, char **argv) {
	char *c;

	if (argc != 2) {
		printf("usage: check <wanted_str>\n");
		return;
	}

	generic(1, "..........");

	generic(2, "1111111111");

	generic(3, ".1.1.1.1.1");

	generic(4, "22222.....");

	generic(5, "1111..2222");

	generic(6, "1111..2222");

	generic(8, argv[1]);
	sleep(20);
}

