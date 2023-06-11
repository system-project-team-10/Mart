
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <math.h>

#include "cordinate.h"

#define BUFFER_SIZE 50

#define ERROR(a,b) ((((a) * (1.1)) < (b)) || (((a) * (0.9) > (b))))

/**
 * ---------------------------------------------------------------
 * |Beac1                                                   Beac2|
 * |                                                             |
 * |                                                             |
 * |                                                             |
 * |                                                             |
 * |                                                             |
 * |                                                             |
 * |                                                             |
 * |                                                             |
 * |                                                             |
 * |                                                             |
 * |                            Beac0                            |
 * ---------------------------------------------------------------
*/

/*
struct Beac beacon[3];

void beacon_init(){
    beacon[0].x=0.;
    beacon[0].y=0.;
    beacon[0].coef=-58;
    beacon[1].x=-3.;
    beacon[1].y=5.;
    beacon[1].coef=-58;
    beacon[2].x=3.;
    beacon[2].y=5.;
    beacon[2].coef=-58;
}
*/

void rssi_passing(double *input, const char*buffer){
    char temp[BUFFER_SIZE];
    strcpy(temp,buffer);
    char *ptr = strtok(temp, " ");    //첫번째 strtok 사용.
    for(int i=0;i<3;i++){
        input[i] = (double) atof(ptr);
        ptr = strtok(NULL," ");
    }
}
double comp[3] = {0.25, 0.5, 1.};
void *location(void *point){
    Point *p = (Point *) point;
    FILE *rssi;
    double input[3];
    double curr[3]={0,0,0};
    int cnt[3]={0,0,0};
    char buffer[BUFFER_SIZE];
    double dist[3];
    //beacon_init();
    
    while(1){
        // get rssi
        rssi = fopen("curr", "r");

        flock(fileno(rssi), LOCK_SH);
        fgets(buffer, BUFFER_SIZE, rssi);
        rssi_passing(input, buffer);

        // 3-count incremental correction algorithm
        for(int i=0;i<3;i++){
            if(input[i]==0.) continue;

            if(ERROR(curr[i],input[i])){
                curr[i] += (input[i]-curr[i]) * comp[cnt[i]]; 
                cnt[i] = (cnt[i]+1)%3;
            }
            else{
                curr[i] = input[i];
                cnt[i] = 0;
            }
        }
        //printf("input rssi : %.2f %.2f %.2f\n",input[0],input[1],input[2]);
        //printf("corre rssi : %.2f %.2f %.2f\n",curr[0],curr[1],curr[2]);
        get_loc(p,curr[0],curr[1],curr[2]);
        printf("%0.4lf %0.4lf\n",p->x,p->y);

        // send coord to other socket
        sleep(1);
    }


    //beacon init

}

void main(){
    Point p;
    
    *location((void *)&p);
}