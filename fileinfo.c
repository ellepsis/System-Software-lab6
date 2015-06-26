#include "fileinfo.h"
#include <sys/types.h>
#include <dirent.h>
#include <stddef.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include "overal_functions.h"

#define STD_ALLOCATE_COUNT 128

int get_dir_info(char *path, char **namesp){
    DIR *dir;
    struct dirent *dir_entry;
    size_t count = 0, len;
    size_t allocated_memory;
    allocated_memory = STD_ALLOCATE_COUNT;
    char *names = NULL;

    names = (char *) malloc(sizeof(char)*STD_ALLOCATE_COUNT);
    if ((dir = opendir(path)) == NULL) {
        return -1;
    }
    while ((dir_entry = readdir(dir))!= NULL){
        len = strlen(dir_entry->d_name);
        if (allocated_memory <= count+len+2) {
            allocated_memory = allocated_memory * 3 / 2;
            names = (char *) realloc(names, allocated_memory * sizeof(char));
            if (names == NULL)
                print_error("Can't realloc memory", 14, errno);
        }
        memcpy(names+count, dir_entry->d_name, len);
        count+=len;
        names[count] = '\n';
        names[count+1] = '\0';
        count += 2;

    }
    closedir(dir);
    *namesp = names;
    return count;
}

void free_names(char *names){
    free(names);
}





