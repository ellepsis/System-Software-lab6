#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "overal_functions.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 11232
#define BUF_SIZE 1024*1024
#define STD_ALLOCATE_COUNT 128

void print_error_close_sock(char* msg, int error, int errnum, int sockfd){
    close(sockfd);
    print_error(msg, error, errnum);
}

int comparator(char* fst, char* snd){
    return strcasecmp(*(char**)fst, *(char**)snd);
}

int sort_string_and_write(char *buff, int buff_ln){
    char **lines;
    int allocated_size = STD_ALLOCATE_COUNT;
    int start_pos = 0, i, lines_count = 0;

    lines = malloc(allocated_size*sizeof(char*));
    for (i = 0; i<buff_ln; i++){
        if (buff[i]=='\0'){
            if(allocated_size<=lines_count) {
                allocated_size = allocated_size * 3 / 2;
                lines = (char**)realloc(lines, allocated_size * sizeof(char *));
                if (lines == NULL)
                    print_error("Can't realloc memory", 14, errno);
            }
            lines[lines_count] = malloc(sizeof(char)*(++i-start_pos));
            memcpy(lines[lines_count++], buff+start_pos, i-start_pos);
            start_pos = i--;
        }
    }
    qsort(lines, lines_count, sizeof(char*), comparator);
    for(i = 0; i< lines_count; i++){
        if (write(STDOUT_FILENO, lines[i], strlen(lines[i]))<0){
            print_error("Can't recive data", 6, errno);
        }
        free(lines[i]);
    }
    free(lines);
}

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in sock_addr;
    ssize_t rcv_data;
    int input_len;
    char *path, *buffer;

    if (argc<3) print_error("Invalid arguments count", 13, 0);
    path = malloc(sizeof(char)*260);
    buffer = malloc(sizeof(char)*BUF_SIZE);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0)
        print_error("Can't create socket", 1, errno);
    if (strchr(*(++argv), ':')!=NULL)
        print_error("Invalid adress", 12, 0);
    if (!strcmp(*argv, "localhost")){
        sock_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
    else {
        if ((sock_addr.sin_addr.s_addr = inet_addr(*argv)) == (unsigned int)-1)
            print_error("Invalid adress", 11, 0);
    }
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(PORT);
    if (connect(sockfd, (struct sockaddr*)&sock_addr, sizeof(sock_addr))<0)
        print_error_close_sock("Can't connect to socket", 3, errno, sockfd);
    while (*(++argv)!=NULL){
        strcpy(path, *argv);
        input_len = strlen(path);
        path[input_len++] = '\r';
        path[input_len++] = '\n';
        path[input_len++] = '\0';
        if (send(sockfd, path, input_len, 0)<0)
            print_error_close_sock("Can't send message", 4, errno, sockfd);
        while ((rcv_data = recv(sockfd, buffer, BUF_SIZE, 0)) < 0) {
            print_error_close_sock("Can't recive data", 5, errno, sockfd);
        }
        if (rcv_data == 0) break;
        printf("%s\n", *argv);
        sort_string_and_write(buffer, rcv_data);
    }
    close(sockfd);
}

