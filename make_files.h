#include "reports.h"
#include<sys/stat.h>

#ifndef MAKE_FILE_H
#define MAKE_FILES_H

int init_district(const char *district_name);
int add_report(const char *district_name,Report *new_report);
void permission_converter(__mode_t mode);
void list_district(const char *district_id);
void view_report(const char *district_id,int targe_id);
void remove_report(const char *district_id,int target_id);
void update_treshhold(const char *district_id,const char *value);
void setup_links(const char *dir_name);
#endif