#include <stdio.h>
#include <math.h>
#define PI 3.14159265358979323846

// 좌표 구조체 정의
typedef struct predictPoint {
double x1;
double y1;
double x2;
double y2;
} predictPoint;

typedef struct Point {
double x;
double y;
} Point;

typedef struct Coefficient1{
double a;
double b;
double c;
} Coefficient1;

// 두 점 사이의 거리를 계산하는 함수
double calculateDistance(Point p1, Point p2) {
double dx = p2.x - p1.x;
double dy = p2.y - p1.y;
return sqrt(dx * dx + dy * dy);
}

double calculateTriangleArea(Point A, Point B, Point C){
    double a = calculateDistance(B, C);
    double b = calculateDistance(C, A);
    double c = calculateDistance(A, B);
    double s = (a + b + c) / 2.0;
    return sqrt(s * (s - a) * (s - b) * (s - c));
}
Coefficient1 calculatedotDistance(Point A, Point B){
    double coeff1=B.y-A.y;
    double coeff2=A.x-B.x;
    double coeff3=B.x*A.y-A.x*B.y;
    double sq=sqrt(coeff1*coeff1+coeff2*coeff2);
    //double distance=fabs(coeff1*x+coeff2*y+coeff3)/sq;
    Coefficient1 eq1;
    eq1.a=coeff1/sq;
    eq1.b=coeff2/sq;
    eq1.c=coeff3/sq;
    return eq1;

}


predictPoint solveQuadraticAndLinearEquations(double a, double b, double c, double d, double e, double f, double g, double h) {
    double x2, y2;
    double x1, y1;
    predictPoint ppoint;
    // 1차식을 통해 x에 대한 식을 구함
    //x = (c - b * y) / a;

    // 2차식을 풀어서 y에 대한 해를 구함
    double discriminant = pow(2*b*c/(a*a)+b*e/a+g, 2)-4*(b*b/(a*a)+f)*(c*c/(a*a)+e*c/a-h);
    if (discriminant < 0) {
        printf("연립 방정식의 해가 존재하지 않습니다.\n");
    }

    double sqrtDiscriminant = sqrt(discriminant);
    y1=((2*b*c/(a*a)+b*e/a+g)+sqrtDiscriminant)/(2*(b*b/(a*a)+f));
    y2=((2*b*c/(a*a)+b*e/a+g)-sqrtDiscriminant)/(2*(b*b/(a*a)+f));


    // 구한 y 값을 1차식에 대입하여 x의 값을 구함
    x1 = (c - b * y1) / a;
    x2 = (c - b * y2) / a;

    ppoint.x1=x1;
    ppoint.x2=x2;
    ppoint.y1=y1;
    ppoint.y2=y2;
    return ppoint;
}


void get_loc(Point *point, double beacon1, double beacon2, double beacon3) {
// 세 점의 좌표
    double distanceAB;
    double distanceAC;
    double distanceBC;
    double spread=3*pow(10, 8); //전파 속도
    double lamda; 
    double fq=2.4*pow(10,9); //주파수
    double dA, dB, dC; //송신자와 수신자의 거리
    double x;
    double y;
    Point loc;
    predictPoint pPoint;
    Point A = { 0, 0 }; // 상황에 맞게 비콘의 위치좌표를 바꾸면 됨
    Point B = { 4, 0 };
    Point C = { 2, 3 };
    distanceAB=calculateDistance(A, B);
    distanceBC=calculateDistance(B, C);
    distanceAC=calculateDistance(C, A);

    dA=spread/4/PI/fq*pow(10, beacon1/20);
    dB=spread/4/PI/fq*pow(10, beacon2/20);
    dC=spread/4/PI/fq*pow(10, beacon3/20);

    
    //연립방정식을 통해 x,y값을 구함
    
    //식1 
    Coefficient1 x1;
    Coefficient1 x2;
    Coefficient1 x3;
    double area=calculateTriangleArea(A,B,C);
    x1=calculatedotDistance(A,B);
    x2=calculatedotDistance(B,C);
    x3=calculatedotDistance(A,C);
    //area=calculateDistance(A,B)*distanceAB/2+calculateDistance(B,C)*distanceBC/2+calculateDistance(A,C)*distanceAC/2
    double a, b, c; //ax+by=c 
    a=x1.a*distanceAB/2+x2.a*distanceBC/2+x3.a*distanceAC/2;
    b=x1.b*distanceAB/2+x2.b*distanceBC/2+x3.b*distanceAC/2;
    c=area-(x1.c*distanceAB/2+x2.c*distanceBC/2+x3.c*distanceAC/2);



    //식2
    //dA=sqrt(pow((x-A.x),2)+pow((y-A.y),2 )
    double d, e, f, g, h; //dx^2+ex+fy^2+gy=h
    d=1.0;
    e=-2*A.x;
    f=1.0;
    g=-2*A.y;
    h=dA-A.x*A.x-A.y*A.y;

    pPoint=solveQuadraticAndLinearEquations(a, b, c, d, e, f, g, h);

    //정답 확인
    Point anw1={pPoint.x1, pPoint.y1};
    Point anw2={pPoint.x2, pPoint.y2};
    double distanceB1=calculateDistance(anw1, B);
    double distanceB2=calculateDistance(anw2, B);
    double gapdB1=fabs(distanceB1-dB);
    double gapdB2=fabs(distanceB2-dB);
    if(gapdB1>gapdB2){
        printf("x = %lf, y = %lf\n", anw2.x, anw2.y);
        *point = anw2;
    }
    else{
        printf("x = %lf, y = %lf\n", anw1.x, anw1.y);
        *point = anw1;
    }
    
    
    return 0;

}


