#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <ctype.h>
#include <pthread.h>

#define BUFFER_SIZE 1024


void trim(char* str) {
    int start = 0;
    int end = strlen(str) - 1;

    // 문자열 시작 부분의 공백 제거
    while (isspace(str[start])) {
        start++;
    }

    // 문자열 끝 부분의 공백 제거
    while (end >= start && isspace(str[end])) {
        end--;
    }

    // 공백을 제거한 문자열 재조정
    int len = end - start + 1;
    memmove(str, str + start, len);
    str[len] = '\0';
}

int main(int argc, char *argv[])
{
    int sock; // 소켓의 fd
    struct sockaddr_in serv_addr; // 서버 정보
    int serv_sock = -1; // 서버 소켓의 fd
    FILE *in;
    char buffer[BUFFER_SIZE];
    int read_buffer = 0;

    char *ip_addr = "";
    int port_addr = 54321;

    int fd;
    char username[BUFFER_SIZE];

    while (1) {
        sock = socket(PF_INET, SOCK_STREAM, 0);
        if (sock != -1) break;
    }

    // 서버 정보 저장
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_addr);
    serv_addr.sin_port = htons(port_addr);

    // 서버와의 연결 시도
    while (1) {
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != -1)
            break;
    }
    
    while (1) { // 유저 인식
        in = fopen("user_rfid.txt", "r");
        flock(fileno(in), LOCK_SH);
        fgets(buffer, BUFFER_SIZE, in);
        if (strcmp(buffer, "nodata\n") == 0) {
            read_buffer = 0;
        }
        else if (read_buffer == 0) {
            if (strncmp(buffer, "User:", 5) == 0) {
                strncpy(username, buffer + 5, strlen(buffer)-5);
                username[strlen(buffer)-6]=0;
                trim(username);
                
                // 유저 이름 서버 전송
                write(sock, username, sizeof(username));

                usleep(5000000);
                break;
            }
            read_buffer = 1;
        }
        flock(fileno(in), LOCK_UN);
        fclose(in);
        usleep(1000000);
    }
    return 0;
}