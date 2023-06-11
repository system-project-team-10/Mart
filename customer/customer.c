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

#define I2C_BUS "/dev/i2c-1"  // I2C 버스 경로
#define I2C_ADDR 0x27         // I2C 주소
#define LCD_CHR  1            // 문자 전송 모드
#define LCD_CMD  0            // 명령 전송 모드
#define LINE1    0x80         // 1번째 줄
#define LINE2    0xC0         // 2번째 줄
#define LCD_BACKLIGHT 0x08    // 0x08 -> ON
#define ENABLE  0b00000100    // Enable 비트
#define I2C_SMBUS_READ 1
#define I2C_SMBUS_BYTE_DATA 2
#define I2C_SMBUS_BLOCK_MAX 32  // SMBus 표준에서 지정된 최대 블록 크기
#define BUFFER_SIZE 50

typedef struct _Item_s Item_s;
void init();
int merge_duplicate_items ();
int i2c_smbus_access(int, char, uint8_t, int, union i2c_smbus_data*);
int i2c_smbus_write_byte(int, int);
void lcd_init();
void clr_lcd();
void lcd_loc(int);
void type_ln(const char *);
void lcd_byte(int, int);
void lcd_toggle_enable(int);
void trim(char*);
void *lcd_work(void*);


typedef struct _Item_s {
    int price;
    int quantity;
    char name[50];
} Item_s;

int fd;
char username[BUFFER_SIZE];
Item_s purchased_item[1000];
int purchased_item_cnt, total;

pthread_t lcd_work_tid, end_work_tid;


int merge_duplicate_items () {
    printf("%s\n", username);
    printf("%d\n", purchased_item_cnt);
    if (username[0] == 0 || purchased_item_cnt == 0) return -1;
    
    for (int i = 0; i < purchased_item_cnt - 1; i++) {
        if (purchased_item[i].price != -1) {
            for (int j = i + 1; j < purchased_item_cnt; j++) {
                if (strcmp(purchased_item[i].name, purchased_item[j].name) == 0) {
                    purchased_item[i].quantity++;
                    purchased_item[j].price = -1; // price가 -1면 다음부터 무시.
                }
            }
        }
    }
    return 0;
}

union i2c_smbus_data
{
    uint8_t byte;
    uint16_t word;
    uint8_t block[I2C_SMBUS_BLOCK_MAX + 2];
};

int i2c_smbus_access(int fd, char rw, uint8_t command, int size, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args;

    args.read_write = rw;
    args.command = command;
    args.size = size;
    args.data = data;
    return ioctl(fd, I2C_SMBUS, &args);
}

int i2c_smbus_write_byte(int fd, int value)
{
    union i2c_smbus_data data;

    if (i2c_smbus_access(fd, I2C_SMBUS_READ, value, I2C_SMBUS_BYTE_DATA, &data))
        return -1;
    else
        return data.byte & 0xFF;
}

void lcd_init()
{
    // LCD 초기화
    lcd_byte(0x33, LCD_CMD);
    lcd_byte(0x32, LCD_CMD);
    lcd_byte(0x06, LCD_CMD);
    lcd_byte(0x0C, LCD_CMD);
    lcd_byte(0x28, LCD_CMD);
    lcd_byte(0x01, LCD_CMD);
    usleep(500);
}

void clr_lcd()
{
    lcd_byte(0x01, LCD_CMD); // 화면 지우기
    lcd_byte(0x02, LCD_CMD); // 커서를 홈 위치로 이동
}

void lcd_loc(int line)
{
    lcd_byte(line, LCD_CMD); // LCD의 현재 위치를 지정된 줄로 이동
}

void type_ln(const char *s)
{
    while (*s)
        lcd_byte(*(s++), LCD_CHR); // 현재 위치에 문자 출력
}

void lcd_byte(int bits, int mode)
{
    // 데이터 핀으로 바이트 전송
    // bits: 전송할 데이터
    // mode: 1은 데이터 전송 모드, 0은 명령 전송 모드
    int bits_high;
    int bits_low;
    bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    i2c_smbus_write_byte(fd, bits_high);
    lcd_toggle_enable(bits_high);

    i2c_smbus_write_byte(fd, bits_low);
    lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits)
{
    // LCD 디스플레이의 enable 핀을 토글
    usleep(500);
    i2c_smbus_write_byte(fd, (bits | ENABLE));
    usleep(500);
    i2c_smbus_write_byte(fd, (bits & ~ENABLE));
    usleep(500);
}

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

void init() {
    for (int i = 0; i < 1000; i++) {
        purchased_item[i].price = 0;
        purchased_item[i].quantity = 0;
        for (int j = 0; j < BUFFER_SIZE; j++)
            purchased_item[i].name[j] = 0;
    }
    for (int i = 0; i < BUFFER_SIZE; i++) {
        username[i] = 0;
    }
    purchased_item_cnt = 0;
    total = 0;
    fd = 0;
}

void *lcd_work (void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    init();
    FILE *in;
    char item[BUFFER_SIZE];
    int price;
    char item_full[BUFFER_SIZE];

    char price_str[BUFFER_SIZE], total_str[BUFFER_SIZE];

    if ((fd = open(I2C_BUS, O_RDWR)) < 0)
    {
        perror("open I2C device failed.\n");
    }

    if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0)
    {
        perror("select I2C device failed.\n");
    }
    char buffer[BUFFER_SIZE];
    int read_buffer = 0;
    clr_lcd();
    lcd_loc(LINE1);
    type_ln("Welcome!");
    lcd_loc(LINE2);
    type_ln("Tag your ID Card");

    while (1) { // 유저 인식
        pthread_testcancel();
        in = fopen("rfid.txt", "r");
        // printf("fopened\n");
        flock(fileno(in), LOCK_SH);
        // printf("get lock success\n");
        fgets(buffer, BUFFER_SIZE, in);
        if (strcmp(buffer, "nodata\n") == 0) {
            read_buffer = 0;
        }
        else if (read_buffer == 0) {
            if (strncmp(buffer, "User:", 5) == 0) {
                strncpy(username, buffer + 5, strlen(buffer)-5);
                username[strlen(buffer)-6]=0;
                trim(username);
                char line2_str[BUFFER_SIZE];
                snprintf(line2_str, BUFFER_SIZE, "%s!", username);
                clr_lcd();
                lcd_loc(LINE1);
                type_ln("Welcome");
                lcd_loc(LINE2);
                type_ln(line2_str);
                usleep(5000000);
                break;
            }
            else if(strncmp(buffer, "Item:", 5) == 0){
                clr_lcd();
                lcd_loc(LINE1);
                type_ln("Tag your");
                lcd_loc(LINE2);
                type_ln("ID Card First.");
                usleep(3000000);
                clr_lcd();
                lcd_loc(LINE1);
                type_ln("Welcome!");
                lcd_loc(LINE2);
                type_ln("Tag your ID Card");
                continue;
            }
            // printf("%s", buffer);
            read_buffer = 1;
        }
        flock(fileno(in), LOCK_UN);
        // printf("unlocked\n");
        fclose(in);
        // printf("fclosed\n\n");

        usleep(1000000);
    }
    clr_lcd();
    lcd_loc(LINE1);
    type_ln("Tag Item please");
    lcd_loc(LINE2);
    type_ln("Total: 0");

    while (1) { // 물품 인식
        pthread_testcancel();
        in = fopen("rfid.txt", "r");
        flock(fileno(in), LOCK_SH);
        fgets(buffer, BUFFER_SIZE, in);
        if (strcmp(buffer, "nodata\n") == 0) {
            read_buffer = 0;
        }
        else if (read_buffer == 0) {
            if (strncmp(buffer, "Item:", 5) == 0) {
                strncpy(item, buffer + 5, strlen(buffer)-5);
                item[strlen(buffer)-6]=0;
                trim(item);
                char *item_start = item;
                char *item_end = strchr(item, ',');
                // item 문자열 추출
                int item_len = item_end - item_start;
                char* item_name = (char*)malloc((item_len + 1) * sizeof(char));
                strncpy(item_name, item_start, item_len);
                item_name[item_len] = '\0';
                
                // 정수 값 추출
                price = atoi(item_end + 1);
                // printf("%d\n", price);
                sprintf(price_str, "%d", price);
                // printf("%s\n", price_str);

                clr_lcd();
                lcd_loc(LINE1);
                char line1_str[BUFFER_SIZE], line2_str[BUFFER_SIZE];
                snprintf(line1_str, BUFFER_SIZE, "Item: %s", item_name);
                snprintf(line2_str, BUFFER_SIZE, "Price: %s", price_str);
                type_ln(line1_str);
                lcd_loc(LINE2);
                type_ln(line2_str);
                usleep(3000000);

                total += price;
                sprintf(total_str, "%d", total);

                clr_lcd();
                lcd_loc(LINE1);
                type_ln("Tag Item please");
                lcd_loc(LINE2);
                snprintf(line2_str, BUFFER_SIZE, "Total: %s", total_str);
                type_ln(line2_str);

                purchased_item[purchased_item_cnt].quantity++;
                purchased_item[purchased_item_cnt].price = price;
                strcpy(purchased_item[purchased_item_cnt].name, item_name);
                purchased_item_cnt++;


                usleep(3000000);

                free(item_name);
            }
            read_buffer = 1;
        }
        flock(fileno(in), LOCK_UN);
        fclose(in);
    }
    pthread_exit(NULL);
}

void *end_work (void *arg) {
    int end = 0;
    scanf("%d", &end);
    printf("END\n");
    pthread_cancel(lcd_work_tid);
    pthread_exit(NULL);

}
int main()
{

    while (1) {
        if (pthread_create(&lcd_work_tid, NULL, lcd_work, NULL) != 0)
            perror("lcd work thread create error\n");
        if (pthread_create(&end_work_tid, NULL, end_work, NULL) != 0)
            perror("end work thread create error\n");

        pthread_join(lcd_work_tid, NULL);
        pthread_join(end_work_tid, NULL);


        int status;
        status = merge_duplicate_items();

        if (status == 0) {
            FILE *file = fopen("purchase_history.txt", "a");
            if (file != NULL) {
                fprintf(file, "User: %s\n", username);
                for (int i = 0; i < purchased_item_cnt; i++) {
                    if (purchased_item[i].price != -1) {
                        fprintf(file, "Item: %s, Quantity: %d, Price: %d, Total: %d\n", purchased_item[i].name, purchased_item[i].quantity, 
                                purchased_item[i].price, purchased_item[i].quantity*purchased_item[i].price);
                    }
                }
                fprintf(file, "Total amount: %d\n", total);
                fprintf(file, "\n");
                fclose(file);
            }
        }
        clr_lcd();
        lcd_loc(LINE1);
        type_ln("Thank you!");
        lcd_loc(LINE2);
        type_ln("See you later.");

        close(fd);
        usleep(5000000);
    }

    return 0;
}