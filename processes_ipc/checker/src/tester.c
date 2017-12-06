#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include "api.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <checker> <test-dat-file>\n", argv[0]);
        return 1;
    }
    int tester_to_chck[2], chck_to_tester[2];
    pipe(tester_to_chck);
    pipe(chck_to_tester);
    pid_t p_checker = fork();
    if (p_checker == -1) {
        errhandler("fork failed");
    }
    if (p_checker > 0) {
        int fdesk_out = tester_to_chck[1];
        int fdesk_in = chck_to_tester[0];
        uint16_t num_of_ques = 0;
        send_cmd(fdesk_out, GET_NUM_OF_QUESTIONS, 0, NULL, 0);
        recv_cmd(fdesk_in, GET_NUM_OF_QUESTIONS, &num_of_ques, NULL);
        char msg[BUFSIZE], answer[PREFSIZE + BUFSIZE];
        request_gettext(fdesk_in, fdesk_out, 0, msg);
        printf("  Today's test topic is \"%s\"\n\n", msg);
        uint16_t len_answer, amount_of_correct = 0;
        for (uint16_t i = 1; i <= num_of_ques; i++) {
            request_gettext(fdesk_in, fdesk_out, i, msg);
            printf("  %" PRIu16 ") %s\n  >  ", i, msg);
            input_answer(answer, &len_answer);
            if (request_checkanswer(fdesk_in, fdesk_out, i, answer, len_answer)) {
                amount_of_correct += 1;
            }
        }
        if (amount_of_correct == num_of_ques) {
            printf("Congratulations! Your knowledge is perfect!\n");
        } else {
            printf("Your result is: %d/%" PRIu16 "\n", amount_of_correct, num_of_ques);
        }
        send_cmd(tester_to_chck[1], EXIT, 0, NULL, 0);
        close(chck_to_tester[0]);
        close(tester_to_chck[1]);
        int status;
        if (waitpid(p_checker, &status, 0) == -1 ) {
            errhandler("waitpid failed");
        }
        if (WIFEXITED(status)) {
            const int es = WEXITSTATUS(status);
            if (es) {
                printf("Checker exited abnormally. Contact your teacher!\n");
            }
        }
        exit(EXIT_SUCCESS);
    } else if (p_checker == 0) {
        dup2(chck_to_tester[1], STDOUT_FILENO);
        close(chck_to_tester[1]);
        dup2(tester_to_chck[0], STDIN_FILENO);
        close(tester_to_chck[0]);
        execl(argv[1], argv[1], argv[2], (char *) 0);
        errhandler("execl");
    } 
}
