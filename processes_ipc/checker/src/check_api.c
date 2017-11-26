#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "check_api.h"
#define bytes_to_u16(HIGH, LOW) \
(((unsigned int) ((unsigned char) HIGH)) & 255) << 8 | \
                  (((unsigned char) LOW) & 255) 

void errhandler(void) {
    perror(NULL);
    exit(EXIT_FAILURE);
}

char read_command(unsigned int *arg) {
    char buf[COMM_LEN];
    int k;
    if ((k = read(0, buf, COMM_LEN)) == -1) {
        return ERR_READ;
    }
    if (k == 0) {
        *arg = 0;
        return EXIT;
    }
    *arg = bytes_to_u16(buf[1], buf[2]);
    return buf[0];
}

static void xor(char *data, char *key, unsigned int length) {
    int key_len = strlen(key), i = 0;
    while (length--) {
        data[i] ^= (key[i % key_len] - 2 * i * (i & 1));
        i++;
    }
}

static int read_token(int fd, char *buf, unsigned int *len) {
    int k;
    char len_buf[ARGSIZE];
    if ((k = read(fd, len_buf, ARGSIZE)) != ARGSIZE) {
        return ERR_READ;
    }
    *len = bytes_to_u16(len_buf[0], len_buf[1]);
    if ((unsigned int) (k = read(fd, buf, *len)) != *len) {
        if (k == -1) {
            errhandler();
        }
        return ERR_READ;
    }
    return 0;
}

static void safe_read_token(int fd, char *buf, unsigned int *len, char *status) {
    int k;
    if ((k = read_token(fd, buf, len)) == ERR_READ) {
        *status = ERR_READ;
    }
}

static void safe_write(int fd, char *buf, unsigned int len, char *status) {
    int k;
    if ((unsigned int) (k = write(fd, buf, len)) != len) {
        if (k == -1) {
            errhandler();
        }
        *status = ERR_WRITE;
    }
}

static int check_answer(char *right, char *answer, unsigned int len) {
    unsigned int i = 0, j = 0;
    while (isspace(answer[j])) {
        j++;
    }
    while (answer[i + j] != '\0') {
        answer[i] = answer[i + j];
        i++;
    }
    answer[i] = '\0';
    i = strlen(answer) - 1;
    while (i > 0 && isspace(answer[i])) {
        i--;
    }
    answer[i + 1] = '\0';
    if (len != strlen(answer)) {
        return 0;
    }
    for (i = 0; i < len; ++i) {
        right[i] = tolower((unsigned char) right[i]);
        answer[i] = tolower((unsigned char) answer[i]);
    }
    return !memcmp(right, answer, len);
}

int exec_command(int fd, char opcode, unsigned int arg, char *key, char *status) {
    *status = OK;
    if (opcode == EXIT) {
        return 0;
    }
    off_t cursor_before = lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);
    unsigned int len, tmp_len;
    char buf[BUFSIZE] = {'\0'}, tmp[BUFSIZE], answer[BUFSIZE] = {'\0'};
    *status = OK; 
    safe_read_token(fd, buf, &len, status);
    if (*status == ERR_READ) {
        return 0;
    }
    switch (opcode) {
        case GET_TEXT:
            while (arg--) {
                safe_read_token(fd, buf, &len, status);
                safe_read_token(fd, tmp, &tmp_len, status);
            }
            xor(buf, key, len);
            if (*status == OK) {
                safe_write(1, buf, len, status);
            }
            break;
        case CHECK_ANSW:
            *status = RIGHT_ANSW;
            safe_read_token(0, answer, &tmp_len, status);
            while (arg--) {
                safe_read_token(fd, tmp, &tmp_len, status);
                safe_read_token(fd, buf, &len, status);
            }
            if (*status != OK) {
                break;
            }
            xor(buf, key, len);
            buf[len] = '\0';
            if (!check_answer(buf, answer, len)) {
                *status = WRONG_ANSW;
            }
            break;
        default:
            *status = ERR_RUNTIME;
            break;
    }
    lseek(fd, cursor_before, SEEK_SET);
    return 0;
}
