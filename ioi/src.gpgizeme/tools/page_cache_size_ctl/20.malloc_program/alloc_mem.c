#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
	int ret;
	long	mem_size_in_mb = 0;
	long	mem_size = 0;
	void * p = (void *)0;

	if (argc <= 1) {
		printf("memory size in MB should be specified as an argument\n");
		return -1;
	}

	mem_size_in_mb = (long) atol(argv[1]);
	printf("mem_size_in_mb: %ld MB\n", mem_size_in_mb);

	mem_size = mem_size_in_mb * 1024 * 1024;
	p = malloc(mem_size);
	if (p == (void *)0) {
		printf("%ld MB memory allocation: failed\n", mem_size_in_mb);
		return -2;
	}
	printf("%ld MB memory allocation: successed\n(address: %lu, size: %ld bytes)\n", mem_size_in_mb, (unsigned long)p, mem_size);

	getchar();

	ret = 0;
	return ret;
}
