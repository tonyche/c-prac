#ifndef MODULE_H_INCLUDED
#define MODULE_H_INCLUDED 

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
    PADDING = 16,
    ARGSIZE = 2,
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

void errhandler(void);

char read_command(uint16_t *arg);

int exec_command(int fd, char opcode, uint16_t arg, char *key);

char *assembly(char opcode, uint16_t arg, char *data, size_t *len);

#endif
