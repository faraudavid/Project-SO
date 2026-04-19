#include "reports.h"
#include<sys/stat.h>

#ifndef MAKE_FILE_H
#define MAKE_FILES_H

int init_district(const char *district_name);
int add_report(const char *district_name,Report *new_report);
void permission_converter(__mode_t mode);
void list_district(const char *district_id);
#endif