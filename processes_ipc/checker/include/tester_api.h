#ifndef MODULE_H_INCLUDED
#define MODULE_H_INCLUDED 

enum {
    BLOCKSIZE = 128,
    SEND_MODE = 0,
    RECV_MODE = 1
};

void print_fatal(int mode, char opcode);

void send_cmd(int fd, char opcode, uint16_t arg, char *data, size_t len_data);

void receive_cmd(int fd, char opcode, uint16_t *result, char *data);

void request_gettext(int fd_in, int fd_out, uint16_t arg, char *data);

void input_answer(char *answer, uint16_t *len_answer);

int request_checkanswer(int fd_in, int fd_out, uint16_t arg, char *answer, uint16_t len);

#endif