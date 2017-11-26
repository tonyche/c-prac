#ifndef MODULE_H_INCLUDED
#define MODULE_H_INCLUDED 

enum {
    OK = 0,
    WRONG_ANSW = 1,
    RIGHT_ANSW = 2,
    BUFSIZE = 65536,
    ARGSIZE = 2,
    ERR_READ = -1,
    ERR_WRITE = -2,
    ERR_RUNTIME = -3,
    COMM_LEN = 3,
    GET_TEXT = 0x01,
    CHECK_ANSW = 0x02, 
    EXIT = 0x00
};

void errhandler(void);

char read_command(unsigned int *arg);

int exec_command(int fd, char opcode, unsigned int arg, char *key, char *status);

#endif