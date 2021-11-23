
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#define SYSCALL_NUM 449

int syscall_mapspages(unsigned long start, unsigned long end, char *buf, size_t size) {
	int ret;
	printf("value is %#lx %ld\n", start, start);
        ret = syscall(SYSCALL_NUM, start, end, buf, size);
        if (0 > ret){
                perror("Failed with");
		return -1;
	}
        return ret;
}

int main(int argc, char **argv)
{
	unsigned long start, end;

	if (argc < 3 || argc > 4) {
		printf("usage: check <start> <end>\n");
		return -1;
	}

	start = (unsigned long) strtol(argv[1], NULL, 16);
	end = (unsigned long) strtol(argv[2], NULL, 16);

	char *c = malloc(2);
	printf("syscall end with: %d\n", syscall_mapspages(start, end, c, 2));

	printf("sleeping");
	sleep(300);
	printf("done sleeping");
}
