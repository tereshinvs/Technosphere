#ifndef CACHE_MAP_FILE_H
#define CACHE_MAP_FILE_H

#include <stdlib.h>

char *map_file(char const *file_name, size_t length);
void unmap_file(char *data, size_t length);

#endif
