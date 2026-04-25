#include "make_files.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
int main(int argc,char **argv) {
    setup_links(".");
    char role[32] = {0},user[32]={0},action[32]={0},district[64]={0},threshold[32]={0};
    int report_id = -1;
    int num_conditons = 0;
    int index_filter_start = 0;


for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            strncpy(role, argv[++i], sizeof(role) - 1);
            role[sizeof(role) - 1] = '\0';
        }
        else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) strcpy(user, argv[++i]);
        else if (strcmp(argv[i], "--add") == 0 && i + 1 < argc) {
            strcpy(action, "add");
            strcpy(district, argv[++i]);
        }
        else if (strcmp(argv[i], "--list") == 0 && i + 1 < argc) {
            strcpy(action, "list");
            strcpy(district, argv[++i]);
        }
        else if (strcmp(argv[i], "--remove") == 0 && i + 2 < argc) {
            strcpy(action, "remove");
            strcpy(district, argv[++i]);
            report_id = strtol(argv[++i], NULL, 10);
        }
        else if (strcmp(argv[i], "--view") == 0 && i + 2 < argc) {
            strcpy(action, "view");
            strcpy(district, argv[++i]);
            report_id = strtol(argv[++i], NULL, 10);
        }
        else if (strcmp(argv[i], "--update_threshold") == 0 && i + 2 < argc) {
            strcpy(action, "update");
            strcpy(district, argv[++i]);
            strcpy(threshold, argv[++i]);
        }
        else if (strcmp(argv[i], "--filter") == 0) {
            if (i+2 > argc) {
                fprintf(stderr,"Error");
                return -1;
            }
            strcpy(action,"filter");
            strcpy(district,argv[++i]);
            index_filter_start = i+1;
            num_conditons = argc - index_filter_start;
            break;

        }
    }

    if(strcmp(action,"add") == 0) {
        if(init_district(district) !=0) {
        fprintf(stderr,"Init failed\n");
        return -1;
    }

Report r;
static int  id = -1;
    r.id  = id;
    id++;
    strncpy(r.inspector,user,sizeof(r.inspector) -1) ;
    printf("Latitude: ");
    scanf("%lf",&r.latitude);
    printf("Longitude: ");
    scanf("%lf",&r.longitude);
    getchar();
    printf("Category: ");
    fgets(r.category,sizeof(r.category),stdin);
    r.category[strcspn(r.category,"\n")]=0;

    printf("Severity (1-5): ");
    scanf("%d",&r.severity);
    getchar();
    printf("Description: ");
    fgets(r.description,sizeof(r.description),stdin);
    r.description[strcspn(r.description,"\n")] = 0;
    r.timestamp=time( NULL);


    if(add_report(district,&r,user,role) != 0) {
        fprintf(stderr,"Error adding the report\n");
        return -1;
    }
    }
    else if (strcmp(action,"list") == 0) {
        list_district(district,user,role);
    }
    else if (strcmp(action,"view") == 0) {
        view_report(district,report_id,user,role);
    }
    else if (strcmp(action,"remove") == 0) {
        if (strcmp(role,"manager") !=0) {
            fprintf(stderr,"Acess Denied\n");
            return -1;
        } else {
        remove_report(district,report_id,user,role);
        }
    }
    else if (strcmp(action,"update") == 0) {
        if (strcmp(role,"manager")!=0) {
            fprintf(stderr,"Access Denied\n");
            return -1;
        }else {
            
            update_threshold(district,threshold,user,role);
    } 
}else if(strcmp(action,"filter") == 0) {
        filter_reposts(district,num_conditons,&argv[index_filter_start]);
    }
    return 0;
}