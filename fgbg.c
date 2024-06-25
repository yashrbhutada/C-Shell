#include "fgbg.h"
#include "prompt.h"

// Brings a background job to foreground and changes state to running
void fg(char *prompt) {
    command cmd = getArgv(prompt);

    if (cmd.num < 2) {
        printf("fg: Too few arguments\n");
        return;
    }

    if (cmd.num > 2) {
        printf("fg: Too many arguments\n");
        return;
    }

    //Convert the read PID to integer
    int job;
    sscanf(cmd.str_vec[1], "%d", &job);

    processInfo *p = getProcessJob(job);

    if (p == NULL) {
        printf("fg: process with job no %d does not exist\n", job);
        return;
    }

    fg_Proc.job_Num = 0;
    fg_Proc.pid = p->pid;
    strcpy(fg_Proc.pName, p->pName);

    delProcess(fg_Proc.pid);
    bg_count--;

    fg_running = true;
    time_t seconds = time(NULL);

    kill(fg_Proc.pid, SIGCONT);

    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(0, fg_Proc.pid);

    int status = 0;
    waitpid(fg_Proc.pid, &status, WUNTRACED);

    tcsetpgrp(0, getpgrp());
    signal(SIGTTOU, SIG_DFL);

    if (WIFSTOPPED(status)) {
        bg_count++;
        numProcess++;

        printf("\n");
        updatePlist(fg_Proc.pid, fg_Proc.pName);
    }

    if (WIFSIGNALED(status))
        printf("\n");

    fg_running = false;
    fg_Proc.pid = -1;

    seconds = time(NULL) - seconds;

    if (seconds > 1)
        printf("# %s for %ld seconds\n", fg_Proc.pName, seconds);
}

// Continues a bacground job if stopped
void bg(char *prompt) {
    command cmd = getArgv(prompt);

    if (cmd.num < 2) {
        printf("bg: Too few arguments\n");
        return;
    }

    if (cmd.num > 2) {
        printf("bg: Too many arguments\n");
        return;
    }

    char *endptr;
    long int job;
    job = strtol(cmd.str_vec[1], &endptr, 10);

    processInfo *p = getProcessJob(job);

    if (p == NULL) {
        printf("bg: process with job no %ld does not exist\n", job);
        return;
    }

    kill(p->pid, SIGCONT);
}