#include <fcntl.h>
#include <getopt.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <wiringPi.h>
#include <softPwm.h>

int MotorPin1 = 4;
int MotorPin2 = 5;
int MotorPin3 = 2;
int MotorPin4 = 3;

int PIN1 = 21;
int PIN2 = 22;
int PIN3 = 23;
int PIN4 = 24;


int ENA = 6;
int ENB = 7;




void setup() {
  int a;
  a = wiringPiSetup();
  // pinMode(ENA, OUTPUT);  // 출력 모드로 설정
  // pinMode(ENB, OUTPUT);  // 출력 모드로 설정
  softPwmCreate(ENA, 0, 255);
  softPwmCreate(ENB, 0, 255);
  pinMode(MotorPin1, OUTPUT);  // 출력 모드로 설정
  pinMode(MotorPin2, OUTPUT);  // 출력 모드로 설정
  pinMode(MotorPin3, OUTPUT);  // 출력 모드로 설정
  pinMode(MotorPin4, OUTPUT);  // 출력 모드로 설정

  pinMode(PIN1, INPUT);  // 입력 모드로 설정
  pinMode(PIN2, INPUT);  // 입력 모드로 설정
  pinMode(PIN3, INPUT);  // 입력 모드로 설정
  pinMode(PIN4, INPUT);  // 입력 모드로 설정


  digitalWrite(MotorPin1, LOW);
  digitalWrite(MotorPin2, LOW);
  digitalWrite(MotorPin3, LOW);
  digitalWrite(MotorPin4, LOW);
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
}


void motorstop() {
  digitalWrite(MotorPin1, LOW);
  digitalWrite(MotorPin2, LOW);
  digitalWrite(MotorPin3, LOW);
  digitalWrite(MotorPin4, LOW);
  // digitalWrite(ENA, LOW);
  // digitalWrite(ENB, LOW);
  softPwmWrite(ENA, 0);
  softPwmWrite(ENB, 0);
}


void motorControl(int pin1, int pin2, int time) {

//  analogWrite(ENABLE,0~255);
  digitalWrite(pin1, HIGH);  // 모터 방향 설정 (최초 방향)
  digitalWrite(pin2, LOW);


  delay(time);  // 일정 시간 동안 모터 구동

  digitalWrite(pin1, LOW);  // 모터 정지
  digitalWrite(pin2, LOW);
}

int main() {
  setup();  // 초기 설정


    // 3초간 직진
  softPwmWrite(ENA, 255);
  digitalWrite(MotorPin1, HIGH); 
  digitalWrite(MotorPin2, LOW);

  softPwmWrite(ENB, 255);
  digitalWrite(MotorPin3, HIGH);  
  digitalWrite(MotorPin4, LOW);
  delay(3000);

  // 3초간 정지
  softPwmWrite(ENA, 255);
  digitalWrite(MotorPin1, LOW); 
  digitalWrite(MotorPin2, LOW);

  softPwmWrite(ENB, 255);
  digitalWrite(MotorPin3, LOW);  
  digitalWrite(MotorPin4, LOW);
  delay(3000);

  // 3초간 후진
  softPwmWrite(ENA, 255);
  digitalWrite(MotorPin1, LOW); 
  digitalWrite(MotorPin2, HIGH);

  softPwmWrite(ENB, 255);
  digitalWrite(MotorPin3, LOW);  
  digitalWrite(MotorPin4, HIGH);
  delay(3000);

  // 3초간 느린 속도로 직진
  softPwmWrite(ENA, 100);
  digitalWrite(MotorPin1, HIGH); 
  digitalWrite(MotorPin2, LOW);

  softPwmWrite(ENB, 100);
  digitalWrite(MotorPin3, HIGH);  
  digitalWrite(MotorPin4, LOW);
  delay(3000);

  motorstop();


for (int i = 0; i< 100; i++)
{


if (digitalRead(PIN1) == LOW) { printf("sensor 1 is LOW\n"); delay(500); }
if (digitalRead(PIN2) == LOW) { printf("sensor 2 is LOW\n"); delay(500); }
if (digitalRead(PIN3) == LOW) { printf("sensor 3 is LOW\n"); delay(500); }
if (digitalRead(PIN4) == LOW) { printf("sensor 4 is LOW\n"); delay(500); }


}


  return 0;
}
