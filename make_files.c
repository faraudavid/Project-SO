#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include "reports.h"
#include<string.h>

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



    int fd = open(file_path,O_WRONLY|O_CREAT|O_APPEND,0664);

    if(fd == -1) {
        perror("Error opening reports.dat\n");
        return -1;
    }
    if (chmod(file_path,0644) == -1) {
        perror ("Error setting report file permissions\n");
        close(fd);
        return -1;
    }

    ssize_t bytes_written = write(fd,new_report,sizeof(Report));

    if (bytes_written != sizeof(Report)) {
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
void view_report(const char *district_id,int targe_id) {
    char file_path[LENGTH];
    snprintf(file_path,sizeof(file_path),"%s/reports.dat",district_id);

    int fd  = open(file_path,O_RDONLY);
    if(fd == -1) {
        perror("Error opening the file");
        return;
    }

    Report r;
    int found = 0;
    while (read (fd,&r,sizeof(Report)) > 0) {
        if (r.id == targe_id) {
            found = 1;
            break;
        }
    }
    if (found) {
        printf("\n--- Report Details [ID: %d] ---\n", r.id);
        printf("Inspector:   %s\n", r.inspector);
        printf("Location:    Lat: %.4f, Long: %.4f\n", r.latitude, r.longitude);
        printf("Category:    %s\n", r.category);
        printf("Severity:    %d\n", r.severity);
        printf("Timestamp:   %s", ctime(&r.timestamp)); // ctime returns a string with \n
        printf("Description: %s\n", r.description);
        printf("-------------------------------\n");

    } else {
        printf("No report found\n");
    }
    close(fd);
}

void remove_report(const char *district_id,int target_id) {
    char file_path[LENGTH];
    snprintf(file_path,sizeof(file_path),"%s/reports.dat",district_id);

    int fd = open(file_path,O_RDWR) ;
    if (fd == -1) {
        perror("Error opening the file");
        return;
    }
    Report r;
    int found = 0;
    off_t offset = -1;
    while (read(fd,&r,sizeof(Report)) > 0)
    {
        if (r.id == target_id) {
            found = 1;
            offset = lseek(fd,0,SEEK_CUR)-sizeof(Report);
            break;
        }
    }
    if (!found) {
        printf("Report id not found");
        close(fd);
        return;
    }
    off_t read_pos = offset + sizeof(Report);
    off_t write_pos = offset;
    while(1) {

        lseek(fd,read_pos,SEEK_SET);
        int bytes_read = read(fd,&r,sizeof(Report));
        if(bytes_read <= 0) break;
        lseek (fd,write_pos,SEEK_SET);
        write(fd,&r,sizeof(Report));
        write_pos +=sizeof(Report);
        read_pos += sizeof(Report);

    }
    if (ftruncate(fd,write_pos) == -1) {
        perror("Error truncating file");
        return;
    } else {
        printf("Report %d removed successfully \n",target_id);
    }
    close(fd);
}
void update_treshhold(const char *district_id,const char *value){
    char cfg_path[LENGTH];
    snprintf(cfg_path,sizeof(cfg_path),"%s/district.cfg",district_id);

    // printf("Updating district %s with value %s\n",cfg_path,value);

    struct stat st;
    if (stat(cfg_path,&st) == -1) {
        perror("Error stating district.cfg");
        return;
    }

    if ((st.st_mode & 0777) != 0640) {
        fprintf(stderr,"Permissions are not 0640");
        return;
    }

    int fd = open(cfg_path,O_WRONLY|O_TRUNC);
    if (fd == -1) {
        perror("Error opening the file");
        return;
    } 
    ssize_t bytes = write(fd,value,sizeof(value));
    // printf("Writing %zu bytes",strlen(value));
    
    if (bytes == -1) {
        perror("Error writing ");
    } else {
        printf("Treshold successfully updated");
    }
    close(fd);
}

int parse_condition(const char *input,char *field,char *op, char *value) {
    return sscanf(input,"%[^:]:%[^:]:%s",field,op,value) == 3;
}
static int compare_int(long a, const char *op , long b) {
    if (strcmp(op,"==") == 0) return a == b;
        if(strcmp(op,"!=") == 0) return a != b;
        if(strcmp(op,"<") == 0) return a < b;
        if(strcmp(op,">") == 0) return a > b;
        if(strcmp(op,"<=") == 0) return a <= b;
        if(strcmp(op,">=") == 0) return a >= b;
        return 0;

}
static int compare_str(const char *a, const char *op, const char *b) {
    int cmp = strcmp(a,b);    
    if (strcmp(op,"==") == 0) return cmp == 0;
        if(strcmp(op,"!=") == 0) return cmp != 0;
        if(strcmp(op,"<") == 0) return cmp < 0;
        if(strcmp(op,">") == 0) return cmp > 0;
        if(strcmp(op,"<=") == 0) return cmp <= 0;
        if(strcmp(op,">=") == 0) return cmp >= 0;
}

int match_condition(Report *r,const char *field,const char* op,const char *value) {
    
    if (strcmp(field,"severity") == 0) {
        return compare_int(r->severity,op,atol(value));
    } 
    else if (strcmp(field,"timestamp") == 0) {
        return compare_int((long)r->timestamp,op,atol(value));
    }
    else if (strcmp(field,"category") == 0) {
        return compare_str(r->category,op,value);
    }
    else if (strcmp(field,"inspector") == 0) {
        return compare_str(r->inspector,op,value);
    }
    return 0;
}