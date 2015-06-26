#ifndef LAB6_FILEINFO_H
#define LAB6_FILEINFO_H

#include <stddef.h>

typedef struct name{
    int len;
    char *name;
}name;

int get_dir_info(char *path, char **names);
void free_names(char *names);

#endif

