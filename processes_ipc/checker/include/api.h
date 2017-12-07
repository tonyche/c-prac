#ifndef MODULE_H_INCLUDED
#define MODULE_H_INCLUDED 

#define bytes_to_u16(HIGH, LOW) \
(((uint16_t) ((uint8_t) HIGH)) & 255) << 8 | \
(((uint8_t) LOW) & 255)

#define HBYTE(A) ((A & (255 << 8)) >> 8)
#define LBYTE(A) (A & 255)

/*
 * Небольшое лирическое отступление на тему того, как красиво и правильно
 * добавлять новые команды для работы с зашифрованным .dat файлом:
 *
 * 1) Сохранять нумерацию (после GET_NUM_OF_QUESTIONS идет номер команды с кодом 0x04 и т.д.)
 * 2) Не забывать увеличивать COMMS_LEN 
*/

enum {
    OK = 0,
    WRONG_ANSW = 1,
    RIGHT_ANSW = 2,
    BUFSIZE = 65536,
    BLOCKSIZE = 128,
    RECV_MODE = 1,
    SEND_MODE = 0,
    PADDING = 16,
    ARGSIZE = 2,
    PREFSIZE = 2,
    ERR_READ = -1,
    ERR_WRITE = -2,
    ERR_RUNTIME = -3,
    COMM_LEN = 3,
    DATA_END = -4,
    GET_TEXT = 0x01,
    CHECK_ANSW = 0x02,
    GET_NUM_OF_QUESTIONS = 0x03,
    COMMS_LEN = 3,
    EXIT = 0x00
};

void errhandler(const char *msg);

char read_command(uint16_t *arg);

int exec_command(int fd, char opcode, uint16_t arg, char *key);

char *assembly(char opcode, uint16_t arg, char *data, size_t *len, size_t len_data);

void print_fatal(int mode, char opcode);

void send_cmd(int fd, char opcode, uint16_t arg, char *data, size_t len_data);

void recv_cmd(int fd, char opcode, uint16_t *result, char *data);

void request_gettext(int fd_in, int fd_out, uint16_t arg, char *data);

void input_answer(char *answer, uint16_t *len_answer, int *iseof);

int request_checkanswer(int fd_in, int fd_out, uint16_t arg, char *answer, uint16_t len);

#endif
