
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#define SYSCALL_NUM 449
#define STARTING_LEN 500

int syscall_mapspages(unsigned long start, unsigned long end, char *buf, size_t size) {
	int ret;
	printf("addresses values are %#lx and %#lx (length is: %#lx)\n", start, end, end-start);
        ret = syscall(SYSCALL_NUM, start, end, buf, size);
        if (0 > ret){
                perror("Failed with");
		return -1;
	}
	printf("return value is %d\n", ret);
        return ret;
}

char * increasing_mapspages(unsigned long start, unsigned long end) {
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
		return c;
	}

}

int main(int argc, char **argv)
{
	unsigned long start, end;
	char *c;

	if (argc < 3 || argc > 4) {
		printf("usage: check <start> <end>\n");
		return -1;
	}

	start = (unsigned long) strtol(argv[1], NULL, 16);
	end = (unsigned long) strtol(argv[2], NULL, 16);

	c = increasing_mapspages(start, end);
	printf("\n\noutput is:\n%s", c);
	free(c);
	return 1;
}
