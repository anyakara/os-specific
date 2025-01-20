#include "csapp.h"


int main(void) {
    pid_t pid;
    if ((pid = Fork()) == 0) {
        Pause();
        printf("control should never reach here!\n");
        exit(0);
    }
    Kill(pid, SIGKILL);
    exit(0);
}

// sending signals with alarm functionality
#ifdef SIGNAL_ASSERTION
#include <unistd.h>
unsigned int alarm(unsigned int seconds);
#endif

#ifdef SIGKILL
// #include <signal.h>
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
sigset_t mask, prev_mask;

ssize_t sio_put1(long v);
ssize_t sio_puts(char s[]);
void sio_error(char s[]);

void sigint_handler(int sig) {
    sio_puts("Caught SIGINT!\n");
    _exit(0);
}

volatile sig_atomic_t flag;

int _sigaction(int signum, struct sigaction *act,
                struct sigaction *oldact); // custom exception type

#endif

// code/ecf/procmaskl.c

/* WARNING: This code is buggy */
void handler (int sig) {
    int olderrno = errno;
    sigset_t mask_all, prev_all;
    pid_t pid;

    sigfillset(&mask_all);

};

