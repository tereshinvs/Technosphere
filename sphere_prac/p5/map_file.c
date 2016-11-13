#include "map_file.h"

#include <sys/mman.h>
#include <fcntl.h>

char *map_file(char const *file_name, size_t length) {
	int fd = open(file_name, O_RDWR | O_CREAT, 0666);
	char *res = (char*)mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (res == MAP_FAILED)
		res = NULL;
	return res;
}

void unmap_file(char *data, size_t length) {
	munmap(data, length);
}
