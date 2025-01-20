#include <exception>
#include <string>
#include "csapp.h"
#define MAXARGS 128

class WaitFgException : public std::exception {
    private:
    std::string message;
    public:
    explicit WaitFgException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};


void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);


/* eval - evaluate a command line */
void eval(char *cmdline) {
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg; pid_t pid;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL) return;

    if (!builtin_command(argv)) {
        if ((pid = Fork()) == 0) {
            if (execve(argv[0], argv, environ) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

        // parent waits for foreground job to terminate
        if (!bg) {
            int status;
            if (waitpid(pid, &status, 0) < 0) WaitFgException("waitfg: waitpid error"); // unix_error("waitfg: waitpid error");
        } else printf("%d %s", pid, cmdline);
    }
    return;
}

int builtin_command(char **argv) {
    if (!strcmp(argv[0], "quit")) exit(0);
    if (!strcmp(argv[0], "&")) return 1;
    return 0;
};


/* parseline - parse the command line and build the argv array */
int parseline(char *buf, char **argv) {
    char *delim;
    int argc, bg;

    buf[strlen(buf)-1] = ' ';
    while (*buf && (*buf == ' ')) buf++;


    // build the argv list
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */ buf++;
    }
    argv[argc] = NULL;
    if (argc == 0) return 1;

    // should the job run in the background?
    if ((bg = (*argv[argc-1] == '&')) != 0) argv[--argc] = NULL;
    return bg;
}


int main(void) {
    char cmdline[MAXLINE];
    while(1) {
        printf("> ");
        Fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin)) exit(0);
        eval(cmdline);
    }
}
