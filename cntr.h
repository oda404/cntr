
#ifndef CNTR_H
#define CNTR_H

#include <stddef.h>

int init();
int run(char **paths, size_t path_count, char **excludes, size_t exclude_count);
int dump();
int destroy();

#endif // !CNTR_H
