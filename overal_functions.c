#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "overal_functions.h"

void print_error(char* message, int error_number, int err_code){
    errno=err_code;
    perror(message);
    exit(error_number);
}

void print_start_message(){
    char *msg = "To exit press ^C or send SIGTERM";
    if (isatty(STDOUT_FILENO) == 1)
        if (write(STDOUT_FILENO, msg, strlen(msg))<0)
            print_error("Can't write to term", 10, errno);
}
