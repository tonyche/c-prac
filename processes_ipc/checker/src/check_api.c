#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "check_api.h"

#define bytes_to_u16(HIGH, LOW) \
(((uint16_t) ((uint8_t) HIGH)) & 255) << 8 | \
            (((uint8_t) LOW) & 255) 

void errhandler(void) {
    perror(NULL);
    exit(EXIT_FAILURE);
}

char read_command(uint16_t *arg) {
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

static void xor(char *data, char *key, uint16_t length) {
    int key_len = strlen(key), i = 0;
    while (length--) {
        data[i] ^= (key[i % key_len] - 2 * i * (i & 1));
        i++;
    }
}

static int read_token(int fd, char *buf, uint16_t *len) {
    int k;
    char len_buf[ARGSIZE];
    if ((k = read(fd, len_buf, ARGSIZE)) != ARGSIZE) {
        off_t old_pos = lseek(fd, 0, SEEK_CUR);
        off_t current_pos = lseek(fd, 0, SEEK_END);
        if (old_pos == current_pos) {
            return DATA_END;
        }
        lseek(fd, old_pos, SEEK_SET);
        return ERR_READ;
    }
    *len = bytes_to_u16(len_buf[0], len_buf[1]);
    if ((k = read(fd, buf, *len)) == -1) {
        errhandler();
    }
    if (((uint16_t) k != *len && k > 0) || k > BUFSIZE - PADDING) {
        return ERR_READ;
    }
    return OK;
}

static void safe_read_token(int fd, char *buf, uint16_t *len, char *status) {
    *status = read_token(fd, buf, len);
}

static void safe_write(int fd, char *buf, uint16_t len, char *status) {
    int k;
    if ((k = write(fd, buf, len)) == -1) {
        errhandler();
    }
    if ((uint16_t) k != len) {
        *status = ERR_WRITE;
    }
}

static int check_answer(char *right, char *answer, uint16_t len) {
    uint16_t i = 0, j = 0;
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

static void write_u16(int fd, uint16_t len, char *status) {
    char buf[ARGSIZE] = {(len & (255 << 8)) >> 8, len & 255};
    if (write(fd, buf, ARGSIZE) != ARGSIZE) {
        *status = ERR_WRITE;
    }
}

static void get_text(int fd, uint16_t arg, char *status, char *key) {
    uint16_t len, tmp_len;
    char buf[BUFSIZE], tmp[BUFSIZE];
    safe_read_token(fd, buf, &len, status);
    while (arg--) {
        safe_read_token(fd, buf, &len, status);
        safe_read_token(fd, tmp, &tmp_len, status);
    }
    if (*status == ERR_READ) {
        return;
    }
    xor(buf, key, len);
    if (*status == OK) {
        write_u16(1, len, status);
        safe_write(1, buf, len, status);
    }
}

static void check_answ(int fd, uint16_t arg, char *status, char *key) {
    uint16_t len, tmp_len;
    char answer[BUFSIZE], tmp[BUFSIZE], buf[BUFSIZE]; 
    *status = WRONG_ANSW;
    safe_read_token(fd, tmp, &tmp_len, status);
    safe_read_token(0, answer, &tmp_len, status);
    answer[tmp_len] = '\0';
    while (arg--) {
        safe_read_token(fd, tmp, &tmp_len, status);
        safe_read_token(fd, buf, &len, status);
    }
    if (*status == ERR_READ) {
        return;
    }
    buf[len] = '\0';
    xor(buf, key, len);
    if (check_answer(buf, answer, len)) {
        *status = RIGHT_ANSW;
    }
}

static void get_num_q(int fd, uint16_t arg, char *status, char *key) {
    uint16_t num = 0, tmp_len;
    char tmp[BUFSIZE];
    key = key; arg = arg;
    safe_read_token(fd, tmp, &tmp_len, status);
    do {
        safe_read_token(fd, tmp, &tmp_len, status);
        if (*status == DATA_END || *status == ERR_READ) {
            break;
        }
        safe_read_token(fd, tmp, &tmp_len, status);
        num++;
    } while (*status != DATA_END && *status != ERR_READ);
    if (*status == DATA_END) {
        *status = OK;
        write_u16(1, num, status);
    }
}

int exec_command(int fd, char opcode, uint16_t arg, char *key) {
    char status = OK;
    if (opcode == EXIT) {
        return OK;
    }
    off_t cursor_before = lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);
    void (*commands[COMMS_LEN]) (int, uint16_t, char *, char *) = {
        get_text, 
        check_answ, 
        get_num_q
    };
    if (opcode <= COMMS_LEN) {
        (*commands[opcode - 1]) (fd, arg, &status, key);
    } else {
        return ERR_RUNTIME;
    }
    safe_write(1, &status, sizeof(status), &status);
    lseek(fd, cursor_before, SEEK_SET);
    return status;
}
