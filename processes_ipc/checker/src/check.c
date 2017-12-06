#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "check_api.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    int ftest = open(argv[1], O_RDONLY);
    if (ftest == -1) {
        errhandler("open()");
    }
    char opcode, *key = "DEFAULT_KEY_MUST_BE_CHANGED";
    uint16_t arg;
    while ((opcode = read_command(&arg)) != EXIT) {
        if (exec_command(ftest, opcode, arg, key) == ERR_RUNTIME) {
            exit(EXIT_FAILURE);
        }
    }
    close(ftest);
    return 0;
}
