#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include "reports.h"

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

int add_report(const char *district_name,Report *new_report) {
    char file_path[LENGTH];

    snprintf(file_path,sizeof(file_path),"%s/reports.dat",district_name);

    struct stat st;

    if (stat(file_path,&st) == 0) {
        if(!S_ISREG(st.st_mode)) {
            fprintf(stderr,"Error: %s is not a regular file\n",file_path);
            return -1;
        }
        mode_t permissions = st.st_mode & 0777;

        if (permissions != 0644) {
            fprintf(stderr,"Incorrect permissions\n");
            chmod(file_path,0644);
        }
    }
    


    int fd = open(file_path,O_WRONLY|O_CREAT|O_APPEND,0664);

    if(fd == -1) {
        perror("Error opening reports.dat\n");
        return -1;
    }
    if (chmod(file_path,0644) == -1) {
        perror ("Error setting report file permissions\n");
        return -1;
    }
    ssize_t bytes_written = write(fd,new_report,sizeof(Report));

    if (bytes_written == -1) {
        perror("Error writing report to file");
        close(fd);
        return -1;
    }
    close(fd);

    printf("Report %d added successfully \n",new_report->id);
    return 0;
}
void permission_converter(mode_t mode) {
    char str[10];

    str[0] = (mode & S_IRUSR) ? 'r' : '-';
    str[1] = (mode & S_IWUSR) ? 'w' : '-';
    str[2] = (mode & S_IXUSR) ? 'x' : '-';

    str[3] = (mode & S_IRGRP) ? 'r' : '-';
    str[4] = (mode & S_IWGRP) ? 'w' : '-';
    str[5] = (mode & S_IXGRP) ? 'x' : '-';

    str[6] = (mode & S_IROTH) ? 'r' : '-';
    str[7] = (mode & S_IWOTH) ? 'w' : '-';
    str[8] = (mode & S_IXOTH) ? 'x' : '-';

    str[9] ='\0';
    printf("%s",str);
}

void list_district(const char *district_id){
    char file_path[256];
    snprintf(file_path,sizeof(file_path),"%s/reports.dat",district_id);

    struct stat st;

    if(stat(file_path,&st) == -1) {
        perror("Error accessing reports.dat");
        return;
    }
    printf("\n ---District: %s ----\n",district_id);
    permission_converter(st.st_mode);
    printf("\nSize: %ld bytes\n",st.st_size);
    printf("Last modification: %s",ctime(&st.st_mtime));

    int fd = open(file_path,O_RDONLY);
    if (fd == -1) {
        perror("Error opening reports file");
        return;
    }
    Report r;
    int count = 0;
    while(read(fd,&r,sizeof(Report)) > 0) {
        printf("ID: %d | Inspector: %s | Severity: %d | Category: %s\n", 
                r.id, r.inspector, r.severity, r.category);
        count++;
    }
    if(count == 0) {
        printf("No reports found\n");
    }
    close(fd);
    printf("--------------\n");
}
