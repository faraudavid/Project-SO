#include "make_files.h"
#include<stdio.h>
int main() {
    const char *district_name = "District1";
    const char *district_name2 = "District2";

    if(init_district(district_name) !=0) {
        fprintf(stderr,"Init failed\n");
        return -1;
    }
    if (init_district(district_name2) != 0){
        fprintf(stderr,"Init failed\n");
        return -1;
    }

    Report r1 = {
    .id = 101,
    .inspector = "Alice Smith",
    .latitude = 45.7489,
    .longitude = 21.2087,
    .category = "Roads",
    .severity = 2,
    .timestamp = time(NULL), 
    .description = "Pothole repair required on main street."
};

Report r2 = {
    .id = 102,
    .inspector = "Bob Jones",
    .latitude = 45.7505,
    .longitude = 21.2112,
    .category = "Sanitation",
    .severity = 3,
    .timestamp = time(NULL),
    .description = "Blocked storm drain due to debris."
};

    if(add_report(district_name,&r1) != 0) {
        fprintf(stderr,"Error adding the report\n");
        return -1;
    }
    if(add_report(district_name,&r2) != 0) {
        fprintf(stderr,"Error adding the report\n");
        return -1;
    }
    list_district(district_name);
    list_district(district_name2);
    
    return 0;
}