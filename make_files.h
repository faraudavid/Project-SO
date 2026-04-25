#include "reports.h"
#include<sys/stat.h>

#ifndef MAKE_FILES_H
#define MAKE_FILES_H

int init_district(const char *district_name);
int add_report(const char *district_name,Report *new_report,const char *user, const char *role);
void permission_converter(__mode_t mode);
void list_district(const char *district_id,const char *user,const char *role);
void view_report(const char *district_id,int targe_id,const char *user,const char *role);
void remove_report(const char *district_id,int target_id,const char *user,const char *role);
void update_threshold(const char *district_id,const char *value,const char *user,const char *role);
void setup_links(const char *dir_name);
void log_operation(const char *district_id,const char *details,const char *role,const char *user,const char *op);
void filter_reposts(const char *district_id, int argc, char *argv[]);
#endif