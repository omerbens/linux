#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#define SYSCALL_NUM 449
#define STARTING_LEN 500
#define PAGE_SIZE 4096

int syscall_mapspages(unsigned long start, unsigned long end, char *buf, size_t size) {
	int ret;
	printf("addresses values are %#lx and %#lx (length is: %#lx or %ld pages)\n", start, end, end-start, (end-start)/PAGE_SIZE);
        ret = syscall(SYSCALL_NUM, start, end, buf, size);
        if (0 > ret){
                perror("Failed with");
		return -1;
	}
	printf("return value is %d\n", ret);
        return ret;
}

void print_increasing_mapspages(unsigned long start, unsigned long end) {
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
		printf("output is:\n%s\n\n", c);
		free(c);
		return;
	}
	
}

int cmd_program(int argc, char **argv) {
	unsigned long start, end;
	char *c;

	if (argc < 3 || argc > 4) {
		printf("usage: check <start> <end>\n");
		return -1;
	}

	start = (unsigned long) strtol(argv[1], NULL, 16);
	end = (unsigned long) strtol(argv[2], NULL, 16);

	print_increasing_mapspages(start, end);
	return 1;
}

int main() {
	char *c;
	unsigned long addr;
	int len;

	len = 10;
	addr = (unsigned long) mmap(NULL, PAGE_SIZE*len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if ((void*)addr == MAP_FAILED) {
		perror("failed mmap");
		return 1;
	}
	print_increasing_mapspages(addr, addr+PAGE_SIZE*len);
	munmap((void*)addr, PAGE_SIZE*len);



	len = 10;
	addr = (unsigned long) mmap(NULL, PAGE_SIZE*len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_LOCKED, -1, 0);
	if ((void*)addr == MAP_FAILED) {
		perror("failed mmap");
		return 1;
	}
	print_increasing_mapspages(addr, addr+PAGE_SIZE*len);
	munmap((void*)addr, PAGE_SIZE*len);
}
