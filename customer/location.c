
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <math.h>

struct Custom{
    char id[10];
    float x,y;
}Custom;

struct Beac{
    float x,y; //coordinate
    float coef;   //coefficient

}Beac;

#define BUFFER_SIZE 50

#define ERROR(a,b) ((((a) * (1.1)) < (b)) || (((a) * (0.9) > (b))))

struct Beac beacon[3];
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
void rssi_passing(float *input, const char*buffer){
    char temp[BUFFER_SIZE];
    strcpy(temp,buffer);
    char *ptr = strtok(temp, " ");    //첫번째 strtok 사용.
    for(int i=0;i<3;i++){
        input[i] = (float) atof(ptr);
        ptr = strtok(NULL," ");
    }
}
float comp[3] = {0.25, 0.5, 1.};
void *location(void *coord){
    struct Custom customer = *(struct Custom *) coord;
    FILE *rssi;
    float input[3];
    float curr[3]={0,0,0};
    int cnt[3]={0,0,0};
    char buffer[BUFFER_SIZE];
    float dist[3];
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
                cnt[i] = (cnt[i]+1)%3;
            }
            else{
                curr[i] = input[i];
                cnt[i] = 0;
            }
        }
        //printf("input rssi : %.2f %.2f %.2f\n",input[0],input[1],input[2]);
        //printf("corre rssi : %.2f %.2f %.2f\n",curr[0],curr[1],curr[2]);

        // calculate distance
        for(int i=0;i<3;i++){
            dist[i] = (powf(10.,(beacon[i].coef - curr[i])/20.));
        }
        // calculate coordinate
        printf("%.2f %.2f %.2f\n",dist[0],dist[1],dist[2]);
        // send coord to other socket
        sleep(1);
    }


    //beacon init

}

void main(){
    struct Custom customer={"0000001",0.,0.};
    
    *location((void *)&customer);
}
