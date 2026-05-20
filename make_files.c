#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include "reports.h"
#include<string.h>
#include<dirent.h>
#include<sys/wait.h>

#define LENGTH 256
#define SIZE_PATH 1024
#define DEFAULT_SEVERITY 4
#define PID_FILE  "/tmp/.monitor_pid"


void setup_links(const char *base_dir) {
    DIR *dir = opendir(base_dir);
    if (!dir) {
        perror("Could not open base directory");
        return;
    }

    struct dirent *entry;
    struct stat st;
    char district_path[SIZE_PATH];
    char report_path[SIZE_PATH + 32];
    char link_name[SIZE_PATH + 32];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if(entry->d_name[0]=='.') continue;
        snprintf(district_path, sizeof(district_path), "%s/%s", base_dir, entry->d_name);

        if (lstat(district_path, &st) == -1) continue;

    
        if (S_ISDIR(st.st_mode)) {
        
            snprintf(report_path, sizeof(report_path), "%s/reports.dat", entry->d_name);

            snprintf(link_name, sizeof(link_name), "active_reports-%s", entry->d_name);

            struct stat lnk_stat;
        
            if (lstat(link_name, &lnk_stat) == 0) {
                if (stat(link_name, &lnk_stat) == -1) {
                    fprintf(stderr, "Warning: Dangling link detected for %s. Recreating...\n", entry->d_name);
                    unlink(link_name); 
                    symlink(report_path, link_name);
                }
            } else {
                if (symlink(report_path, link_name) == 0) {
                    printf("Link created: %s -> %s\n", link_name, report_path);
                }
            }
        }
    }
    closedir(dir);
}
void log_operation(const char *district_id,const char *details,const char *role,const char *user,const char *op) {
    char log_path[SIZE_PATH];
    snprintf(log_path,sizeof(log_path),"%s/logged_district",district_id);
    int fd = open(log_path,O_WRONLY | O_APPEND | O_CREAT,0644);
    if (fd != -1) {
        chmod(log_path,0644);
        time_t now = time(NULL);
        char *ts = ctime(&now);
        if(ts != NULL) {
        ts[strlen(ts)-1]='\0';
        }
        dprintf(fd,"[%s] DETAILS %s |ROLE %s | USER %s | OP %s\n",ts,details,role,user,op);
    close(fd);
    }
}
int get_id(const char *district) {
    char path[128];
    snprintf(path,sizeof(path),"%s/reports.dat",district);
    int fd = open(path,O_RDONLY);
    if (fd == -1) {
        return 0;
    }
    off_t file_size = lseek(fd,0,SEEK_END);
    if(file_size <=0) {
        close(fd);
        return 0;
    }
    lseek(fd,-sizeof(Report),SEEK_END);
    Report last;
    read(fd,&last,sizeof(Report));
    return last.id+1;
}
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
        dprintf(fd,"%d",DEFAULT_SEVERITY);
        close(fd);
    }
    fd = open(log_path,O_CREAT|O_WRONLY,0644);
    if (fd != -1) {
        chmod(log_path,0644);
        close(fd);
    }
    printf("District '%s' \n",district_name);
    setup_links(".");
    return 0;
} 

static int notify_monitor(void) {
    int fd = open(PID_FILE,O_RDONLY);
    if(fd == -1) {
        return -1;
    }
    char buff [16] = {0};
    ssize_t n = read(fd,buff,sizeof(buff));
    close(fd);

    if(n <= 0)
        return -1;
    
    pid_t monitor_pid = (pid_t)atoi(buff);
    if (monitor_pid <= 0)
        return -1;

    if(kill(monitor_pid,SIGUSR1) == -1)
        return -2;


    return 0;
}

int add_report(const char *district_name,Report *new_report,const char *user,const char *role) {
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
    char details[1024];
    snprintf(details,sizeof(details),"ADDED report %d",new_report->id);
    // printf("%s ->%s ->%s-> %s",district_name,details,role,user);
    log_operation(district_name,details,role,user,"ADD");
    close(fd);
    printf("Report %d added successfully \n",new_report->id);
    int rc = notify_monitor();
    char log_path[SIZE_PATH];
    snprintf(log_path,sizeof(log_path),"%s/logged_district",district_name);
    int lfd = open(log_path,O_WRONLY|O_APPEND|O_CREAT,0644);
    if(lfd != -1) {
        time_t now = time(NULL);
        char *ts = ctime(&now);
        if(ts) ts[strlen(ts) - 1] ='\0';

        if(rc == 0) {
            dprintf(lfd,
                    "[%s] MONITOR notified successfully (SIGUSR1 sent) for report %d\n",
                    ts, new_report->id);

        } else {
            dprintf(lfd,"MONITOR could not be informed of report");

        }
        close(lfd);
    }
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

void list_district(const char *district_id,const char *user,const char *role){
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
    char details[64];
    snprintf(details,sizeof(details),"LIST %s ",district_id);
    log_operation(district_id,details,role,user,"LIST");
    close(fd);
    printf("--------------\n");
}
void view_report(const char *district_id,int targe_id,const char *user,const char *role) {
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
    char details[64];
    snprintf(details,sizeof(details),"VIEW report %d",targe_id);
    log_operation(district_id,details,role,user,"VIEW");
    close(fd);
}

void remove_report(const char *district_id,int target_id,const char *user,const char *role) {
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
    char details[64];
    snprintf(details,sizeof(details),"REMOVED report %d",target_id);
    log_operation(district_id,details,role,user,"REMOVE");
    close(fd);
}
void update_threshold(const char *district_id,const char *value,const char *user,const char *role){
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
    ssize_t bytes = write(fd,value,strlen(value));
    // printf("Writing %zu bytes",strlen(value));
    
    if (bytes == -1) {
        perror("Error writing ");
    } else {
        printf("Treshold successfully updated");
    }
    char details[64];
    snprintf(details,sizeof(details),"UPDATED  %s",district_id);
    log_operation(district_id,details,role,user,"UPDATE");
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
return 0;
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

void filter_reposts(const char *district_id,int num_cond,char **cond_string) {
    char file_path[256];
    snprintf(file_path,sizeof(file_path),"%s/reports.dat",district_id);
    int fd = open(file_path,O_RDONLY);
    if(fd == -1) {
        perror("Error opening reports file");
        return;
    }
    Report r;
    int found_any = 0;

    while(read(fd,&r,sizeof(Report)) > 0) {
        int matches_all = 1;
        for(int i = 0 ; i  < num_cond; i++) {
            char field[32],op[4],value[64];
            if(parse_condition(cond_string[i],field,op,value)) {
                if(!match_condition(&r,field,op,value)) {
                    matches_all = 0;
                    break;
                }
            }
        }
        if(matches_all) {
            printf("ID: %d | Inspector: %s | Severity: %d | Category: %s\n", 
                   r.id, r.inspector, r.severity, r.category);
            found_any = 1;
        }
    }
    if(found_any == 0) {
        printf("No reports match\n");
    }
    close(fd);
    
}

int get_threshold(const char *district) {
    char path[128];
    snprintf(path,sizeof(path),"%s/district.cfg",district);
    int  fd = open(path,O_RDONLY);
    int threshold = 0;
    if (fd != -1) {
        char buff[16];
        int bytes_read = read(fd,buff,sizeof(buff));
        if(bytes_read > 0) {
            return atoi(buff);
        }
    close(fd);
    }
    return 0;
}
void check_and_alert(int severity, const char *district) {
    int threshold = get_threshold(district);
    if (severity >= threshold) {
        printf("\n****************************************\n");
        printf("  [!] ALERT: HIGH SEVERITY DETECTED!\n");
        printf("  Report Severity: %d | Threshold: %d\n", severity, threshold);
        printf("****************************************\n\n");
    }
}

void remove_district(const char* district_id) {
    
    
    if (strstr(district_id,".") != NULL || strstr(district_id,"/") != NULL) {
        return;
    }
    
    char dir_path[256];
    char link_path[256];
    snprintf(link_path,sizeof(link_path),"active_reports-%s",district_id);
    int pid = fork();
    if(pid < 0) {
        perror("Fork failed");
        return;
    }
    if(pid == 0) {
               
               execlp("rm","rm","-rf",district_id,NULL);   
    perror("execlp failed");
    exit(1);
    }  else {
        int status = 0;
        waitpid(pid,&status,0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            if(unlink(link_path) == 0) {
                printf("District %s and its link removed successfully\n",district_id);
            }else {
                    perror("Something failde at deleting symbolic link");
            }
        } else {
            printf("Failed to delete the specified directory %s\n", district_id);
        }
    }
}

