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

void generic(int testnum, char *s, int check_mmap) {
	long PAGE_SIZE = sysconf(_SC_PAGESIZE);
	int len = (int) strlen(s);
	char *ptr;

	ptr = mmap(0, PAGE_SIZE*len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (1 == check_mmap && ptr == MAP_FAILED) {
		perror("failed mmap");
		return;
	}

	for (int i=0; i<len; i++) {
		switch (s[i])
		{
			case '.':
				break;
			case '1':
				mlock(&ptr[PAGE_SIZE*i], PAGE_SIZE);
		    		break;
			case '2':
				ptr[PAGE_SIZE*i] = '0';
				break;
			// TODO 3-X
			default:
				printf("unkown value: %c\n", s[i]);
				return;
		}
	}
	printf("test%d \"%s\"\n", testnum, s);
	print_maps((unsigned long) ptr, (unsigned long)(ptr+PAGE_SIZE*len));
	
	if (1 == check_mmap && -1 == munmap(ptr, PAGE_SIZE*len))
		printf("failed to munmap\n");
}

void main(int argc, char **argv) {
	char *c;

	if (argc > 3 || argc == 1 || strlen(argv[1]) != 1) {
		printf("usage: check <digit_test_num> [wanted_str_for_test8]\n");
		return;
	}

	// for test6
	char *test6 = malloc(2001);
	if (NULL == test6){
		printf("failed malloc");
		return;
	}
	for (int i=0; i<2000; i++) {test6[i]='.';}

	char *test9 = malloc(9999999);
	if (NULL == test9){
		printf("failed malloc");
		return;
	}
	for (int i=0; i<9999998; i++) {test9[i]='.';}


	switch (argv[1][0])
	{
		case '1':
			generic(1, "..........", 1);
			break;
		case '2':		
			generic(2, "1111111111", 1);
	    		break;
		case '3':
			generic(3, ".1.1.1.1.1", 1);
			break;
		case '4':
			generic(4, "22222.....", 1);
			break;
		case '5':
			generic(5, "1111..2222", 1);
			break;
		case '6':
			generic(6, test6, 1);
			break;
		case '7':
			// TODO: stack
			break;
		case '8':
			if (3 == argc)
				generic(8, argv[2], 1);
			else
				printf("usage: check <digit_test_num> [wanted_str_for_test8]\n");
			break;
		case '9':
			generic(9, test9, 0);
			break;
		default:
			printf("unkown digit_test_num: %s\n", argv[1]);
			break;
	}

	free(test6);
	free(test9);
}

