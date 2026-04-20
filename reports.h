#ifndef REPORTS_H
#define REPORTS_H

#include<time.h>

#define MAX_STR 32
#define MAX_DESC 256

typedef struct {
    int id;
    char inspector[MAX_STR];
    double latitude;
    double longitude;
    char category[MAX_STR];
    int severity;
    time_t timestamp;
    char description[MAX_DESC];
} Report;

#endif
