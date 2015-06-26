#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include "overal_functions.h"
#include "fileinfo.h"

#define MAX_THREADS_COUNT 10
#define SCNT_COUNT 5
#define SDT_COUNT 7
#define PORT 11232
#define MAX_NAME_LEN 260
#define ERROR_OPEN_MSG "Can't read dir\n"
#define exit_recive() close(sockfd); free(tmpval); return;

pthread_mutex_t mutex_start, mutex_socket;
size_t waiting_threads, created_threads;
pthread_cond_t conditional;
int input_sock_fd;
int sockfd = -1;
pthread_t threads[MAX_THREADS_COUNT];
char isWorked[MAX_THREADS_COUNT];

void print_error_close_socket(char* msg, int error, int ernum, int sockfd){
    close(sockfd);
    while (--created_threads>0)
        pthread_cancel(threads[created_threads]);
    pthread_mutex_destroy(&mutex_socket);
    pthread_mutex_destroy(&mutex_start);
    print_error(msg, error, ernum);
}

void start_receive(int sockfd){
    char buff[MAX_NAME_LEN];
    char *names;
    char *tmpval;
    ssize_t i;

    while (1) {
        if ((i = recv(sockfd, buff, MAX_NAME_LEN, 0)) <= 0) {
            exit_recive();
        }
        buff[i] = '\0';
        for (i = 0; i < MAX_NAME_LEN; i++) {
            if (buff[i] == '\0') {
                if (buff[--i] == '\n') buff[i] = '\0';
                if (buff[--i] == '\r') buff[i] = '\0';
                i = -1;
                break;
            };
        }
        if (!strcmp(buff, "exit")){
            exit_recive();
        }
        if (i != -1) {
            exit_recive();
        }
        i = get_dir_info(buff, &names);
        if (i < 0){
            if(send(sockfd, ERROR_OPEN_MSG, strlen(ERROR_OPEN_MSG)+1, 0)<0)
                print_error_close_socket("Can't send msg", 10, errno, sockfd);
            continue;
        }
        if (send(sockfd, names, i, 0)<0)
            print_error_close_socket("Can't send msg", 10, errno, sockfd);
        free_names(names);
    }
}

void *process_request(void *arg){
    int sockfd;
    char *val;
    val = arg;
    while (1){
        pthread_mutex_lock(&mutex_start);
        waiting_threads++;
        *val = 1;
        pthread_cond_wait(&conditional, &mutex_start);
        waiting_threads--;
        sockfd = input_sock_fd;
        *val = 0;
        pthread_mutex_unlock(&mutex_start);
        pthread_mutex_unlock(&mutex_socket);
        start_receive(sockfd);
    }
}

void sig_handler(int sigvalue){
    if (sockfd!=-1) close(sockfd);
}

int main(int argc, char **argv){
    size_t i;
    struct sockaddr_in sock_addr;

    signal(SIGINT, &sig_handler);
    signal(SIGTERM, &sig_handler);
    if (pthread_mutex_init(&mutex_start, NULL)!=0 ||
            pthread_mutex_init(&mutex_socket, NULL)!=0){
        print_error("Can't initialize mutex", 2, errno);
    }
    while (created_threads < SCNT_COUNT){
        if (pthread_create(
                &threads[created_threads], NULL, process_request,
                isWorked+created_threads++)!=0)
            print_error("Can't create thread", 1, errno);
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        print_error("Can't create a socket", 3, errno);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(PORT);
    sock_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sockfd, (struct sockaddr*) &sock_addr, sizeof(sock_addr))<0)
        print_error_close_socket("Can't bind socket", 4, errno, sockfd);
    if (listen(sockfd, MAX_THREADS_COUNT)<0)
        print_error_close_socket("Can't listen socket", 5, errno, sockfd);
    while (1){
        if (waiting_threads<=0) {
            if (created_threads < MAX_THREADS_COUNT){
                if (pthread_create(&threads[created_threads], NULL,
                               process_request, isWorked+created_threads++)!=0)
                    print_error("Can't create thread", 7, errno);
            }
        }
        pthread_mutex_lock(&mutex_start);
        while (1) {
            if (waiting_threads > SDT_COUNT) {
                for (i = 0; i < MAX_THREADS_COUNT; i++) {
                    if (isWorked[i] == 1) pthread_cancel(threads[i]);
                }
            }
            else break;
        }
        pthread_mutex_unlock(&mutex_start);
        pthread_mutex_lock(&mutex_socket);
        if ((input_sock_fd = accept(sockfd, NULL, NULL))<0)
            print_error_close_socket("Can't accept data", 6, errno, sockfd);
        if (waiting_threads <= 0) close(input_sock_fd);
        else pthread_cond_signal(&conditional);
    }
}

