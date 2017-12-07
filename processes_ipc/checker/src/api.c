#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include "api.h"

#define write_errcode write_msg(STDOUT_FILENO, -1, status)

//LCOV_EXCL_START
void errhandler(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
//LCOV_EXCL_STOP

char read_command(uint16_t *arg) {
    char buf[COMM_LEN];
    int k;
    if ((k = read(STDIN_FILENO, buf, COMM_LEN)) == -1) {
        return ERR_READ;
    }
    if (k == 0) {
        *arg = 0;
        return EXIT;
    }
    if (k < 3) {
        return ERR_READ;
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
        return ERR_READ;
    }
    *len = bytes_to_u16(len_buf[0], len_buf[1]);
    if ((k = read(fd, buf, *len)) == -1) {
        errhandler("read_token");
    }
    if (fd != STDIN_FILENO) {
        off_t old_pos = lseek(fd, 0, SEEK_CUR);
        off_t current_pos = lseek(fd, 0, SEEK_END);
        lseek(fd, old_pos, SEEK_SET);
        if (old_pos == current_pos) {
            return DATA_END;
        }
    }
    if (((uint16_t) k != *len) || k > BUFSIZE - PADDING) {
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
        errhandler("safe_write");
    }
    if ((uint16_t) k != len) {
        *status = ERR_WRITE;
    }
}

static int check_answer(char *right, char *answer, uint16_t len) {
    uint16_t i = 0, j = 0;
    if (strlen(answer) == 0) {
        if (strlen(right) == 0) {
            return 1;
        }
        else {
            return 0;
        }
    }
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
    char buf[ARGSIZE] = {HBYTE(len), LBYTE(len)};
    if (write(fd, buf, ARGSIZE) != ARGSIZE) {
        *status = ERR_WRITE;
    }
}

static void write_msg(int fd, uint16_t msg, char *status) {
    write_u16(fd, msg, status);
    if (*status == ERR_WRITE) {
        errhandler("write_msg");
    }
}

static void get_text(int fd, uint16_t arg, char *status, char *key) {
    uint16_t len, tmp_len;
    char buf[BUFSIZE], tmp[BUFSIZE];
    safe_read_token(fd, buf, &len, status);
    while (arg-- && *status == OK) {
        safe_read_token(fd, buf, &len, status);
        safe_read_token(fd, tmp, &tmp_len, status);
    }
    xor(buf, key, len);
    if (*status != ERR_READ) {
        write_u16(STDOUT_FILENO, len, status);
        safe_write(STDOUT_FILENO, buf, len, status);
    } else {
        write_errcode;
    }
}

static void check_answ(int fd, uint16_t arg, char *status, char *key) {
    uint16_t len = 0, tmp_len = 0;
    char answer[BUFSIZE], tmp[BUFSIZE], buf[BUFSIZE]; 
    safe_read_token(fd, tmp, &tmp_len, status);
    safe_read_token(STDIN_FILENO, answer, &tmp_len, status);
    if (arg == 0) {
        *status = ERR_RUNTIME;
        return;
    }
    answer[tmp_len] = '\0';
    while (arg-- && *status == OK) {
        safe_read_token(fd, tmp, &tmp_len, status);
        safe_read_token(fd, buf, &len, status);
    }
    buf[len] = '\0';
    xor(buf, key, len);
    if (*status == ERR_READ) {
        write_errcode;
        return;
    }
    if (check_answer(buf, answer, len)) {
        *status = RIGHT_ANSW;
    } else {
        *status = WRONG_ANSW;
    }
    write_msg(STDOUT_FILENO, bytes_to_u16(0x00, *status), status);
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
        write_u16(STDOUT_FILENO, num, status);
    } else {
        write_errcode;
    }
}

char *assembly(char opcode, uint16_t arg, char *data, size_t *len, size_t len_data) {
    *len = sizeof(opcode) + sizeof(arg);
    char *code;
    if (!(code = malloc(*len))) {
        errhandler("memory err at assembly()");
    }
    code[0] = opcode;
    code[1] = (char) HBYTE(arg);
    code[2] = (char) LBYTE(arg);
    if (data) {
        *len += len_data;
        if (!(code = realloc(code, *len))) {
            errhandler("memory err at assembly()");
        }
        memcpy(code + *len - len_data, data, len_data);
    }
    return code;
}

int exec_command(int fd, char opcode, uint16_t arg, char *key) {
    char status = OK;
    if (opcode == ERR_READ) {
        return ERR_RUNTIME;
    }
    off_t cursor_before = lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);
    void (*commands[COMMS_LEN]) (int, uint16_t, char *, char *) = {
        get_text, 
        check_answ, 
        get_num_q
    };
    if ((unsigned int) opcode <= COMMS_LEN) {
        (*commands[opcode - 1]) (fd, arg, &status, key);
    } else {
        return ERR_RUNTIME;
    }
    lseek(fd, cursor_before, SEEK_SET);
    return status;
}

//LCOV_EXCL_START
void print_fatal(int mode, char opcode) {
    //TODO KILL CHILD TOO
    const char *msg[] = {
        "Fatal while sending ",
        "Error while receiving "
    };
    const char *cmds[] = {
        "get_text",
        "check_answ",
        "get_num_q"
    };
    fprintf(stderr, "%s%s\n", msg[!!mode], cmds[opcode - 1]);
    exit(EXIT_FAILURE);
}
//LCOV_EXCL_STOP

void send_cmd(int fd, char opcode, uint16_t arg, char *data, size_t len_data) {
    size_t c_len = 0;
    int k;
    char *cmd;
    if (!(cmd = assembly(opcode, arg, data, &c_len, len_data))) {
        goto Err;
    }
    if ((k = write(fd, cmd, COMM_LEN)) != COMM_LEN) {
        free(cmd);
        if (k == -1) {
            errhandler("err send");
        }
        goto Err;
    }
    c_len -= COMM_LEN;
    if (data) {
        size_t blocks = c_len / BLOCKSIZE, i = 0;
        k = BLOCKSIZE;
        while (blocks && k == BLOCKSIZE) {
            k = write(fd, &cmd[COMM_LEN + BLOCKSIZE * i], BLOCKSIZE);
            i++;
            blocks--;
        }
        if (blocks) {
            goto Err;
        }
        size_t mod = c_len % BLOCKSIZE;
        k = write(fd, &cmd[COMM_LEN + BLOCKSIZE * i], mod);
        if ((unsigned) k != mod) {
            goto Err;
        }
    }
    free(cmd);
    return;
Err:
    free(cmd);
    print_fatal(SEND_MODE, opcode);
}

void recv_cmd(int fd, char opcode, uint16_t *result, char *data) {
    int k;
    char prefix[PREFSIZE];
    if ((k = read(fd, prefix, PREFSIZE)) != PREFSIZE) {
        if (k == -1) {
            errhandler("err receive");
        }
        goto Err;
    }
    uint16_t res;
    if (result) {
        *result = bytes_to_u16(prefix[0], prefix[1]);
    }
    res = bytes_to_u16(prefix[0], prefix[1]);
    if (prefix[1] == ERR_READ) {
        goto Err;
    }
    if (opcode == GET_TEXT) {
        if ((k = read(fd, data, res)) != res) {
            if (k == -1) {
                errhandler("err receive");
            }
            goto Err;
        }
        data[res] = '\0';
    }
    return;
Err:
    print_fatal(RECV_MODE, opcode);
}

void request_gettext(int fd_in, int fd_out, uint16_t arg, char *data){
    send_cmd(fd_out, GET_TEXT, arg, NULL, 0);
    recv_cmd(fd_in, GET_TEXT, NULL, data);
}

void input_answer(char *answer, uint16_t *len_answer, int *iseof) {
    if (!fgets(&answer[PREFSIZE], BUFSIZE - PREFSIZE, stdin)) {
        if (errno) {
            perror("fgets failed");
            exit(EXIT_FAILURE);
        }
        *iseof = 1;
    }
    *len_answer = (uint16_t) strlen(&answer[PREFSIZE]) - 1;
    answer[0] = HBYTE(*len_answer);
    answer[1] = LBYTE(*len_answer);
    answer[*len_answer + PREFSIZE] = '\0';
}

int request_checkanswer(int fd_in, int fd_out, uint16_t arg, char *answer, uint16_t len) {
    uint16_t response;
    send_cmd(fd_out, CHECK_ANSW, arg, answer, len + PREFSIZE);
    recv_cmd(fd_in, CHECK_ANSW, &response, NULL);
    switch (response) {
        case RIGHT_ANSW:
            return 1;
        case WRONG_ANSW:
            return 0;
        default:
            print_fatal(SEND_MODE, CHECK_ANSW);
    };
    return 1;
}
