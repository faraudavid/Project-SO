#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>



#define PID_FILE ".monitor_pid"

volatile sig_atomic_t keep_running = 1;


void handle_sigint(int sig) {
    printf("Monitor received SIGINT.\n");
    keep_running = 0;
}
void handle_sigusr1(int sig) {
    printf("Notification received. A new report's been added\n");
}

int main() {
    int fd = open(PID_FILE,O_WRONLY|O_TRUNC |O_CREAT,0644);
    if(fd == -1) {
        printf("Error opening the file");
        return -1;
    }
    char pid_str[16];
    snprintf(pid_str,sizeof(pid_str),"%d",getpid());
    write(fd,pid_str,strlen(pid_str));
    printf("Monitor started with PID :%s\n",pid_str);
    close(fd);

    struct sigaction sa_int;
    memset(&sa_int,0,sizeof(sa_int));
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;

        if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("monitor_reports: sigaction SIGINT");
        unlink(PID_FILE);
        return 1;
    }
        struct sigaction sa_usr1;
    memset(&sa_usr1, 0, sizeof(sa_usr1));
    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = SA_RESTART;  

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("monitor_reports: sigaction SIGUSR1");
        unlink(PID_FILE);
        return 1;
    }

   
    while(keep_running) {
        pause();
    }
    if(unlink(PID_FILE) == 0) {
        printf("Goodbye\n");
    }
return 0;
}