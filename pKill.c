#include "pKill.h"
#include "prompt.h"

// Signals the process with the given job id
void pKill(char *prompt) {
    command cmd = getArgv(prompt);

    int job;
    int sig;

    sscanf(cmd.str_vec[1], "%d", &job);
    sscanf(cmd.str_vec[2], "%d", &sig);

    processInfo *p = getProcessJob(job);

    if (p == NULL) {
        printf("sig: process with job no %d does not exist\n", job);
        return;
    }

    kill(p->pid, sig);
}
