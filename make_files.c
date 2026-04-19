#include<stdio.h>
#include<errno.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#define LENGTH 256


int init_district(const char *district_name) {
    if (mkdir(district_name,0750) == -1) {
        if (errno != EEXIST) {
            perror("Error creating directory");
            return -1;
        }
    }
    if (chmod(district_name,0750) == -1) {
        perror("Error setting directory permissions");
        return -1;
    }
    char report_path[LENGTH],config_path[LENGTH],log_path[LENGTH];
    snprintf(report_path,sizeof(report_path),"%s/reports.dat",district_name);
    snprintf(config_path,sizeof(config_path),"%s/district.cfg",district_name);
    snprintf(log_path,sizeof(log_path),"%s/logged_district",district_name);
    int fd = open(report_path,O_CREAT|O_WRONLY,0664);
    if (fd != -1) {
        chmod(report_path,0664);
        close(fd);
    }

    fd = open(config_path,O_CREAT|O_WRONLY,0640);
    if (fd != -1) {
        chmod(config_path,0640);
        close(fd);
    }

    fd = open(log_path,O_CREAT|O_WRONLY,0644);
    if (fd != -1) {
        chmod(log_path,0644);
        close(fd);
    }
    printf("District '%s' \n",district_name);
    return 0;
} 