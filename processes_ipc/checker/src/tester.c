#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include "api.h"

#define allow_sigint sigprocmask(SIG_UNBLOCK, &blockset, 0)
#define deny_sigint sigprocmask(SIG_BLOCK, &blockset, 0)

uint16_t amount_of_correct;
int tester_to_chck[2], chck_to_tester[2];
pid_t p_checker;

void exit_wait(void) {
    printf("\nYour result is: %" PRIu16 "\n", amount_of_correct);
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
}

void sigint_handler(int signo) {
    signo = signo;
    printf("Bye!\n");
    exit_wait();
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <checker> <test-dat-file>\n", argv[0]);
        return 1;
    }
    sigset_t blockset;
    sigemptyset(&blockset);
    sigaddset(&blockset, SIGINT);
    signal(SIGINT, sigint_handler);      
    pipe(tester_to_chck);
    pipe(chck_to_tester);
    p_checker = fork();
    if (p_checker == -1) {
        errhandler("fork failed");
    }
    if (p_checker > 0) {
        int fdesk_out = tester_to_chck[1];
        int fdesk_in = chck_to_tester[0];
        uint16_t num_of_ques = 0;
        char msg[BUFSIZE], answer[PREFSIZE + BUFSIZE];
        deny_sigint;
        send_cmd(fdesk_out, GET_NUM_OF_QUESTIONS, 0, NULL, 0);
        recv_cmd(fdesk_in, GET_NUM_OF_QUESTIONS, &num_of_ques, NULL);
        request_gettext(fdesk_in, fdesk_out, 0, msg);
        printf("Today's test topic is \"%s\"\n\n", msg);
        allow_sigint;
        uint16_t len_answer;
        int iseof = 0;
        amount_of_correct = 0;
        for (uint16_t i = 1; i <= num_of_ques; i++) {
            deny_sigint;
            request_gettext(fdesk_in, fdesk_out, i, msg);
            printf("%" PRIu16 ") %s\n", i, msg);
            allow_sigint;
            input_answer(answer, &len_answer, &iseof);
            if (iseof) {
                exit_wait();
            }
            deny_sigint;
            if (request_checkanswer(fdesk_in, fdesk_out, i, answer, len_answer)) {
                amount_of_correct += 1;
            }
            allow_sigint;
        }
        if (amount_of_correct == num_of_ques) {
            printf("Congratulations! Your knowledge is perfect!\n");
        } 
        exit_wait();
    } else if (p_checker == 0) {
        //LCOV_EXCL_START
        dup2(chck_to_tester[1], STDOUT_FILENO);
        close(chck_to_tester[1]);
        dup2(tester_to_chck[0], STDIN_FILENO);
        close(tester_to_chck[0]);
        execl(argv[1], argv[1], argv[2], (char *) 0);
        errhandler("execl");
        //LCOV_EXCL_STOP
    } 
}
