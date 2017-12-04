#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "check_api.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <checker> <test-dat-file>\n", argv[0]);
        return 1;
    }
    int tester_to_chck[2], chck_to_tester[2];
    char status;
    pipe(tester_to_chck);
    pipe(chck_to_tester);
    pid_t p_checker = fork();
    if (p_checker == -1) {
        perror("fork()");
        exit(EXIT_FAILURE);
    }
    if (p_checker > 0) {
        size_t c_len;
        char result[ARGSIZE + 1];
        char *command = assembly(GET_NUM_OF_QUESTIONS, 0, NULL, &c_len);
        write(tester_to_chck[1], command, c_len);
        read(chck_to_tester[0], result, sizeof(result));
        printf("%d %d %d\n", result[0], result[1], result[2]);
        free(command);
        close(chck_to_tester[0]);
        close(tester_to_chck[1]);
        int status;
        if (waitpid(p_checker, &status, 0) == -1 ) {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(status)) {
            const int es = WEXITSTATUS(status);
            printf("exit status was %d\n", es);
        }
        exit(EXIT_SUCCESS);
    } else if (p_checker == 0) {
        dup2(chck_to_tester[1], STDOUT_FILENO);
        close(chck_to_tester[1]);
        dup2(tester_to_chck[0], STDIN_FILENO);
        close(tester_to_chck[0]);
        execl(argv[1], strrchr(argv[1], '/') + sizeof(**argv), argv[2], (char *) 0);
        perror("execl");
        exit(EXIT_FAILURE);
    } 
    return 0;
}
