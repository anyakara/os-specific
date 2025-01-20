#include "csapp.h"
#define n 2

int main(void)
{
    int status, i;
    pid_t pid;

    for (i = 0; i < n; i++) {
        if ((pid = Fork()) == 0) exit(100+1);
    }

    while ((pid == waitpid(-1, &status, 0)) > 0) {
        if (WIFEXITED(status))
            printf("child terminated with exit status");
        else
            printf("child terminated abnormally");
    }

    if (errno != ECHILD) unix_error("waitpid error");
    exit(0);
}