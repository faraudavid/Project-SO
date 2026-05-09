#include <stdio.h>
#include<unistd.h>
#include <stdlib.h>
#include <string.h>
#include<fcntl.h>
#include<sys/types.h>
#include "reports.h"

int main(int argc,char *argv[]) {
    if(argc <2) {
        printf("Error");
        return -1;
    }
    char *distrcit_id = argv[1] ;
    char path[125];
    snprintf(path,sizeof(path),"%s/reports.dat",distrcit_id);
    Report r ;

    int fd = open(path,O_RDONLY);
    if(fd == -1) {
        return 1;
    }
    int sum = 0;

    while(read(fd,&r,sizeof(Report)) > 0) {
        sum += r.severity;
    }
    close(fd);
    printf("District %s | Total : %d \n",distrcit_id,sum);

    return 0;

}