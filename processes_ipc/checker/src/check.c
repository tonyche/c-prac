#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "check_api.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    int ftest = open(argv[1], O_RDONLY);
    if (ftest == -1) {
        errhandler();
    }
    char opcode, status;
    unsigned int arg;
    while ((opcode = read_command(&arg)) != EXIT) {
        exec_command(ftest, opcode, arg, "DEFAULT_KEY_MUST_BE_CHANGED", &status);
        write(1, &status, 1);
        if (status == ERR_RUNTIME) {
            goto Err_exec;
        }
    }
    close(ftest);
    return 0;
Err_exec:
    close(ftest);
    return 1;
}
