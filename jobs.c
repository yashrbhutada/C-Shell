#include "jobs.h"
#include "prompt.h"

// Comparison function for job names
int comProc(const void *e1, const void *e2) {
    processInfo *a = (struct processInfo *)e1;
    processInfo *b = (struct processInfo *)e2;

    if (strcmp(a->pName, b->pName) == 0)
        return a->job_Num > b->job_Num;

    return strcmp(a->pName, b->pName);
}

// Lists all background jobs
void jobs(char *prompt) {
    command cmd = getArgv(prompt);

    bool rFlag = false;
    bool sFlag = false;

    for (int i = 1; i < cmd.num; ++i) {
        if (cmd.str_vec[i][1] == 'r')
            rFlag = true;
        else if (cmd.str_vec[i][1] == 's')
            sFlag = true;
        else {
            printf("jobs: unrecognized option '%s'\n", cmd.str_vec[i]);
            return;
        }
    }

    bool pRun = !(rFlag ^ sFlag) | rFlag;
    bool pStop = !(rFlag ^ sFlag) | sFlag;

    if (bg_count == 0)
        return;

    processInfo p[bg_count];
    processInfo *temp = pHead;

    int i = 0;
    while (temp != NULL) {
        p[i] = *temp;

        temp = temp->next;
        i++;
    }

    char procFile[256];
    qsort(p, bg_count, sizeof(struct processInfo), comProc);

    int buf_size = 10240;
    char *buffer = (char *)calloc(buf_size, sizeof(char));

    for (int i = 0; i < bg_count; ++i) {
        char *string_pid;

        int len = snprintf(NULL, 0, "%d", p[i].pid);
        string_pid = (char *)calloc(len, sizeof(char));
        sprintf(string_pid, "%d", p[i].pid);

        strcpy(procFile, "/proc/");
        strcat(procFile, string_pid);
        strcat(procFile, "/stat");

        int proc_fd = open(procFile, O_RDONLY);

        if (proc_fd == -1)
            printf("pinfo: process with pid = %s does not exist\n", string_pid);

        read(proc_fd, buffer, 10240);
        command params = getArgv(buffer);

        if (strcmp(params.str_vec[2], "T") == 0 && !pStop)
            continue;

        if (strcmp(params.str_vec[2], "T") != 0 && !pRun)
            continue;

        printf("[%d] ", p[i].job_Num);

        if (strcmp(params.str_vec[2], "T") == 0)
            printf("Stopped ");
        else
            printf("Running ");

        printf("%s ", p[i].pName);
        printf("[%d]\n", p[i].pid);
    }
}
