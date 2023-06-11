
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <math.h>

#include "calcul.h"

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


typedef struct {
    double x;
    double y;
    double coef;
} Beacon;

Beacon beacon[3];

void beacon_init(){
    beacon[0].x=0.;
    beacon[0].y=-1.;
    beacon[0].coef=35.68997669;
    beacon[1].x=5;
    beacon[1].y=0.;
    beacon[1].coef=38.781272589;
    beacon[2].x=0.;
    beacon[2].y=5.;
    beacon[2].coef=46.79792746;
}
double OFFSET;
double PATH_LOSS_EXP;
//#define OFFSET -58
//#define PATH_LOSS_EXP 2.5

void calculateScannerPosition(double rssi1, double rssi2, double rssi3) {
    double d1 = pow(10, (rssi1 + beacon[0].coef) / (-10.0 * PATH_LOSS_EXP)); // RSSI를 거리로 변환 (패스로스 공식 사용)
    double d2 = pow(10, (rssi2 + beacon[1].coef) / (-10.0 * PATH_LOSS_EXP));
    double d3 = pow(10, (rssi3 + beacon[2].coef) /  (-10.0 * PATH_LOSS_EXP));
    printf("rssi : %0.2lf %0.2lf %0.2lf\n",rssi1,rssi2,rssi3);
    printf("dist : %0.2lf %0.2lf %0.2lf\n",d1,d2,d3);

    // 삼변측량 알고리즘을 사용하여 스캐너의 좌표 계산
    double A = 2 * (beacon[1].x - beacon[0].x);
    double B = 2 * (beacon[1].y - beacon[0].y);
    double C = d1 * d1 - d2 * d2 - beacon[0].x * beacon[0].x + beacon[1].x * beacon[1].x - beacon[0].y * beacon[0].y + beacon[1].y * beacon[1].y;
    double D = 2 * (beacon[2].x - beacon[1].x);
    double E = 2 * (beacon[2].y - beacon[1].y);
    double F = d2 * d2 - d3 * d3 - beacon[1].x * beacon[1].x + beacon[2].x * beacon[2].x - beacon[1].y * beacon[1].y + beacon[2].y * beacon[2].y;
    double scannerX = (C * E - F * B) / (E * A - B * D);
    double scannerY = (C * D - A * F) / (B * D - A * E);

    printf("Scanner의 좌표: (%lf, %lf)\n", scannerX, scannerY);
}

void rssi_passing(double *input, const char*buffer){
    char temp[BUFFER_SIZE];
    strcpy(temp,buffer);
    char *ptr = strtok(temp, " ");    //첫번째 strtok 사용.
    for(int i=0;i<3;i++){
        input[i] = (double) atof(ptr);
        ptr = strtok(NULL," ");
    }
}
# define NCOUNT 5
double comp[NCOUNT] = {0.1,0.2,0.4, 0.65, 1.};
void *location(void *point){
    Point *p = (Point *) point;
    FILE *rssi;
    double input[3];
    double curr[3]={0,0,0};
    int cnt[3]={0,0,0};
    char buffer[BUFFER_SIZE];
    double dist[3];
    beacon_init();
    
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
                cnt[i] = (cnt[i]+1)%NCOUNT;
            }
            else{
                curr[i] = input[i];
                cnt[i] = 0;
            }
        }
        //printf("input rssi : %.2f %.2f %.2f\n",input[0],input[1],input[2]);
        //printf("corre rssi : %.2f %.2f %.2f\n",curr[0],curr[1],curr[2]);
        calculateScannerPosition(curr[0],curr[1],curr[2]);
        //printf("%0.4lf %0.4lf\n",p->x,p->y);

        // send coord to other socket
        sleep(1);
    }


    //beacon init

}

void main(int argc, char *argv[]){
    Point p;
    if(argc != 2){
        exit(-1);
    }
    PATH_LOSS_EXP = atof(argv[1]);
    *location((void *)&p);
}
