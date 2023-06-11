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

char usernames[100][20] = {"Sangwon Ko", "Jinhwan No", "Sangheon Jeon", "Gaeon Kim"};
int user_count = 4;

// enter는 customer <-> server
// exit은 door -> server
// end_customer는 server -> customer

pthread_t enter_work_tid, exit_work_tid, end_customer_work_tid;


void *end_customer_work (void *arg) {
    char *username = *(char*)arg;

    int serv_sock, clnt_sock = -1;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    FILE *receive_username;
    char buffer[BUFFER_SIZE];
    // TODO
    int port=23451;
    
    while (1) {
        serv_sock = socket(PF_INET, SOCK_STREAM, 0);
        printf("end_customer_work socket...\n");
        if (serv_sock != -1) break;
    }

    // serv_addr 구성
    memset(&serv_addr, 0 , sizeof(serv_addr));
    // address family, IPv4
    serv_addr.sin_family = AF_INET;
    // ip를 할당하는데 server에 존재하는 랜카드 중 사용가능 한 것으로 이루어짐.
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // port 할당, htons = host to network
    serv_addr.sin_port = htons(port);
    
    
    while (1) {
        if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) != -1) break;
        printf("end_customer_work binding...\n");
    }
    
    printf("[*] end_customer_work wait for customer...\n");
    while (1) {
        if (listen(serv_sock, 5) != -1) break;
    }
    clnt_addr_size = sizeof(clnt_addr);
    while (1) {
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock != -1) break;
    }  
    printf("[*] end_customer_work customer connected\n");
    write(clnt_sock, username, sizeof(username));

    shutdown(clnt_sock, SHUT_RDWR);
    close(serv_sock);
    close(clnt_sock);

    pthread_exit(NULL);

}


void *enter_work (void *arg) {
    int serv_sock, clnt_sock = -1;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    FILE *receive_username;
    char buffer[BUFFER_SIZE];
    // TODO
    int port=12345;

    while (1) {
        serv_sock = socket(PF_INET, SOCK_STREAM, 0);
        printf("enter work socket...\n");
        if (serv_sock != -1) break;
    }

    // serv_addr 구성
    memset(&serv_addr, 0 , sizeof(serv_addr));
    // address family, IPv4
    serv_addr.sin_family = AF_INET;
    // ip를 할당하는데 server에 존재하는 랜카드 중 사용가능 한 것으로 이루어짐.
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // port 할당, htons = host to network
    serv_addr.sin_port = htons(port);
    
    
    while (1) {
        if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) != -1) break;
        printf("enter work binding...\n");
    }

    while (1) {
        printf("[*] enter work wait for customer...\n");
        while (1) {
            if (listen(serv_sock, 5) != -1) break;
        }
        clnt_addr_size = sizeof(clnt_addr);
        while (1) {
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
            if (clnt_sock != -1) break;
        }  
        printf("[*] enter work customer connected\n");

        receive_username = fopen("receive_username.txt", "wb");

        while (1) {
            ssize_t bytes_received;
            while (1) {
                bytes_received = read(clnt_sock, buffer, BUFFER_SIZE);
                if (bytes_received != -1) break;
            }
            fwrite(buffer, sizeof(char), bytes_received, receive_username);
            if (bytes_received < BUFFER_SIZE) break;
        }
        fclose(receive_username);
        
        receive_username = fopen("receive_username.txt", "r");
        char username[50];
        fgets(username, BUFFER_SIZE, receive_username);
        fclose(receive_username);
        int match = 0;
        FILE *match_file;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(username, usernames[i]) == 0) {
                // user가 있다고 customer한테 보내야함
                match = 1;
                match_file = fopen("match.txt", "rb");

                while (1) {
                    size_t bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, match_file);
                    if (bytes_read > 0) {
                        while (1) {
                            if (write(clnt_sock, buffer, bytes_read) != -1) break;
                        }
                    }else break;
                }
                printf("[*] enter work sent match\n");
                break;
            }
        }
        if (match == 0) {
            // list에 user가 없음, 없다고 customer한테 보내야함
            match_file = fopen("not_match.txt", "rb");
            while (1) {
                size_t bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, match_file);
                if (bytes_read > 0) {
                    while (1) {
                        if (write(clnt_sock, buffer, bytes_read) != -1) break;
                    }
                }else break;
            }
            printf("[*] enter work sent not_match\n");
        }
        fclose(match_file);
        shutdown(clnt_sock, SHUT_RDWR);
        close(clnt_sock);
        printf("[*] enter work session closed\n\n");
    }
    
}

void *exit_work (void *arg) {
    int serv_sock, clnt_sock = -1;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    FILE *receive_username;
    char buffer[BUFFER_SIZE];
    // TODO
    int port=54321;


    while (1) {
        serv_sock = socket(PF_INET, SOCK_STREAM, 0);
        printf("exit work socket...\n");
        if (serv_sock != -1) break;
    }

    // serv_addr 구성
    memset(&serv_addr, 0 , sizeof(serv_addr));
    // address family, IPv4
    serv_addr.sin_family = AF_INET;
    // ip를 할당하는데 server에 존재하는 랜카드 중 사용가능 한 것으로 이루어짐.
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // port 할당, htons = host to network
    serv_addr.sin_port = htons(port);
    
    
    while (1) {
        if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) != -1) break;
        printf("exit work binding...\n");
    }

    while (1) {
        printf("[*] exit work wait for door...\n");
        while (1) {
            if (listen(serv_sock, 5) != -1) break;
        }
        clnt_addr_size = sizeof(clnt_addr);
        while (1) {
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
            if (clnt_sock != -1) break;
        }  
        printf("[*] exit work door connected\n");

        receive_username = fopen("receive_username_door.txt", "wb");

        while (1) {
            ssize_t bytes_received;
            while (1) {
                bytes_received = read(clnt_sock, buffer, BUFFER_SIZE);
                if (bytes_received != -1) break;
            }
            fwrite(buffer, sizeof(char), bytes_received, receive_username);
            if (bytes_received < BUFFER_SIZE) break;
        }
        fclose(receive_username);
        printf("exit work username -> %s\n", receive_username);

        // 여기서 받은 username을 customer에게 주어야함.
        if (pthread_create(&end_customer_work_tid, NULL, end_customer_work, (void*)&receive_username) != 0)
            perror("end customer work thread create error\n");
        pthread_join(end_customer_work_tid, NULL);

        shutdown(clnt_sock, SHUT_RDWR);
        close(clnt_sock);
        printf("[*] exit work session closed\n\n");
    }
    
}

int main()
{
    if (pthread_create(&enter_work_tid, NULL, enter_work, NULL) != 0)
        perror("enter work thread create error\n");
    if (pthread_create(&exit_work_tid, NULL, exit_work, NULL) != 0)
        perror("enter work thread create error\n");
    

    pthread_join(enter_work_tid, NULL);
    pthread_join(exit_work_tid, NULL);

    return 0;
}