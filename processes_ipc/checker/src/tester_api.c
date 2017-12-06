#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include "check_api.h"
#include "tester_api.h"

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
            errhandler();
        }
        goto Err;
    }
    c_len -= COMM_LEN;
    if (data && len_data != 0) {
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
        if (k != mod) {
            goto Err;
        }
    }
    free(cmd);
    return;
Err:
    free(cmd);
    print_fatal(SEND_MODE, opcode);
}

void receive_cmd(int fd, char opcode, uint16_t *result, char *data) {
    int k;
    char prefix[PREFSIZE];
    if ((k = read(fd, prefix, PREFSIZE)) != PREFSIZE) {
        if (k == -1) {
            errhandler();
        }
        goto Err;
    }
    uint16_t res;
    if (result) {
        *result = bytes_to_u16(prefix[0], prefix[1]);
    }
    res = bytes_to_u16(prefix[0], prefix[1]);
    if (res == ERR_READ) {
        goto Err;
    }
    if (opcode == GET_TEXT) {
        if ((k = read(fd, data, res)) != res) {
            if (k == -1) {
                errhandler();
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
    receive_cmd(fd_in, GET_TEXT, NULL, data);
}

void input_answer(char *answer, uint16_t *len_answer) {
    if (!fgets(&answer[PREFSIZE], BUFSIZE, stdin) && !errno) {
        perror("fgets failed")
        exit(EXIT_FAILURE);
    }
    *len_answer = (uint16_t) strlen(&answer[PREFSIZE]) - 1;
    answer[0] = HBYTE(*len_answer);
    answer[1] = LBYTE(*len_answer);
    answer[*len_answer + PREFSIZE] = '\0';
}

int request_checkanswer(int fd_in, int fd_out, uint16_t arg, char *answer, uint16_t len) {
    uint16_t response;
    send_cmd(fd_out, CHECK_ANSW, arg, answer, len + PREFSIZE);
    receive_cmd(fd_inin, CHECK_ANSW, &response, NULL);
    switch (response) {
        case ERR_READ:
            print_fatal(SEND_MODE, CHECK_ANSW);
        case RIGHT_ANSW:
            return 1;
        case WRONG_ANSW:
            return 0;
    };
}