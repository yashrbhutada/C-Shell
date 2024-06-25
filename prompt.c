#include "prompt.h"
#include "foreground.h"
#include "background.h"
#include "autocomplete.h"

typedef struct termios termios;
termios orig_termios;

void die(const char *s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");

    atexit(disableRawMode);

    termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

// Function to tokenize the arguments with space and tab-space
command getArgv(char *prompt) {
    char *token;
    char delim[] = {' ', '\t'};

    int i = 0;
    command cmd;
    token = strtok(prompt, delim);

    // Continuously tokenizing till we get a NULL
    while (token) {
        cmd.str_vec[i] = token;
        token = strtok(0, delim);
        i++;
    }

    cmd.str_vec[i] = NULL;
    cmd.num = i;
    return cmd;
}

// Prints the username and system
void printNames() {
    printf(GREEN "%s@%s" RESET ":" BLUE "%s" RESET "$ ", userName, sysName, mod_path);
}

// Takes the user input
char *getPrompt() {
    fflush(stdout);

    char c;
    char *prompt = (char *)calloc(MAX_ARG_LEN, sizeof(char));

    setbuf(stdout, NULL);
    enableRawMode();

    memset(prompt, '\0', MAX_ARG_LEN);

    // Pointer to the input memory
    int p = 0;

    while (read(STDIN_FILENO, &c, 1)) {
        if (ctrl_c) {
            memset(prompt, '\0', MAX_ARG_LEN);
            p = 0;
            ctrl_c = false;
        }

        if (c == '\n')
            break;

        // CTRL + D
        if (c == 4) {
            if (p == 0) {
                printf("\n");
                exit(0);
            }
            else continue;
        }

        // Backspace
        if (c == 127) {
            if (p > 0) {
                if (prompt[p - 1] == 9) {
                    for (int i = 0; i < 7; i++) {
                        printf("\b");
                    }
                }

                prompt[--p] = '\0';
                printf("\b \b");
            }

            continue;
        }

        if (c == 9) {
            int t = p-1;

            //Fetch the previous command before tab
            while (t >= 0 && prompt[t] != ' ') {
                t--;
            }

            t++;
            char file[256] = {'\0'};
            for (int i = 0; i < p; ++i) {
                file[i] = prompt[t];
                t++;
            }

            int len = strlen(file);

            int k = autocomplete(file);

            if (k == 1) {
                while (len < strlen(file)) {
                    prompt[p] = file[len];
                    printf("%c", prompt[p]);
                    p++;
                    len++;
                }
            }
            else {
                while (len < strlen(file)) {
                    prompt[p] = file[len];
                    p++;
                    len++;
                }

                printNames();
                printf("%s", prompt);
            }

            continue;
        }

        if (iscntrl(c)) continue;

        prompt[p] = c;
        printf("%c", c);
        p++;
    }

    disableRawMode();
    printf("\n");

    return prompt;
}

// Separates the commands on the basis of semicolon (;) and executes each of them seperately
void parseString(char *prompt) {
    int len = strlen(prompt);
    prompt[len] = ';';
    prompt[len + 1] = '\0';

    char cmd[256][256] = {'\0'};

    int j = 0, k = 0;

    for (int i = 0; i < strlen(prompt); ++i) {
        if (prompt[i] == ';') {
            k = 0;
            j++;
        }
        else {
            cmd[j][k] = prompt[i];
            k++;
        }
    }

    // Execute the command after each string is separated (;)
    for (int i = 0; i < j; ++i) {
        execCommand(cmd[i]);
    }
}

// Separates the background and foreground commands on the basis of apersand (&) and run them appropriately
void execCommand(char *prompt) {
    int len = strlen(prompt);   
    prompt[len] = ' ';
    prompt[len + 1] = '\0';

    char *token;
    char delim[] = {'&'};
    // Tokenise based on & for foreground and background
    token = strtok(prompt, delim);

    int j = 0;
    command backgrd;

    while (token) {
        backgrd.str_vec[j] = (char *)malloc(strlen(token) * sizeof(char));
        backgrd.str_vec[j] = token;
        token = strtok(NULL, delim);
        j++;
    }

    backgrd.num = j;

    int forkRet;

    // Forking the background processes and executing them
    for (int i = 0; i < j - 1; ++i) {
        numProcess++;

        forkRet = fork();

        if (forkRet == -1) {
            printf("Error: Process fork failed\n");
            return;
        }
        else if (forkRet == 0) {
            setpgrp();
            background(backgrd.str_vec[i]);
        }
        else {
            bg_count++;

            // Printing the pid of newly created background process and storing its name
            printf("[%d] %d\n", numProcess, forkRet);
            updatePlist(forkRet, backgrd.str_vec[i]);
        }
    }

    stdin_save = dup(STDIN_FILENO);
    stdout_save = dup(STDOUT_FILENO);

    // Executing foreground process
    foreground(backgrd.str_vec[j - 1]);

    dup2(stdin_save, STDIN_FILENO);
    dup2(stdout_save, STDOUT_FILENO);

    close(stdin_save);
    close(stdout_save);
}

// Updates the process list when a new background process is created
void updatePlist(int pid, char *prompt) {
    time_t dur = time(NULL);
    int len = strlen(prompt);

    if (prompt[len-1] == ' ') {
        prompt[len - 1] = '\0';
        len -= 1;
    }

    int i = 0;

    while (i < len && (prompt[i] == ' ' || prompt[i] == '\t'))
        i++;

    if (pHead == NULL) {
        pHead = (processInfo *)calloc(1, sizeof(processInfo));

        pHead->job_Num = numProcess;
        pHead->pid = pid;
        strcpy(pHead->pName, prompt + i);
        pHead->seconds = dur;
        pHead->next = NULL;

        pNext = pHead;
    }
    else {
        pNext->next = (processInfo *)calloc(1, sizeof(processInfo));
        pNext = pNext->next;

        pNext->job_Num = numProcess;
        pNext->pid = pid;
        strcpy(pNext->pName, prompt + i);
        pNext->seconds = dur;
        pNext->next = NULL;
    }
}

// Deletes the process name of the given process id from our stored list
void delProcess(int pid) {
    processInfo *temp = pHead;

    if (pHead->pid == pid) {
        pHead = pHead->next;
        free(temp);

        if (pHead == NULL)
            numProcess = 0;

        return;
    }

    while (temp->next != NULL) {
        if (temp->next->pid == pid) {
            processInfo *remove = temp->next;
            temp->next = temp->next->next;
            free(remove);
            return;
        }

        temp = temp->next;
    }

    if (pHead == NULL)
        numProcess = 0;
}

// Returns the process name of the given process id
processInfo *getProcess(int pid) {
    processInfo *temp = pHead;

    while (temp) {
        if (temp->pid == pid)
            return temp;

        temp = temp->next;
    }
}

// Returns the process name of the given job id
processInfo *getProcessJob(int job) {
    processInfo *temp = pHead;

    while (temp != NULL) {
        if (temp->job_Num == job)
            return temp;

        temp = temp->next;
    }
}

// Signal handler function to handle child death
void end_bg_process(int sig) {
    int stat;
    pid_t bg_pid = waitpid(-1, &stat, WNOHANG);

    if (bg_pid == fg_Proc.pid)
        return;

    if (bg_pid > 0) {
        processInfo *temp = getProcess(bg_pid);
        time_t dur = time(NULL) - temp->seconds;

        int i = 0;
        int l = 0;
        char cmd[MAX_ARG_LEN];
        while (temp->pName[i] != ' ' && temp->pName[i] != '\t' && temp->pName[i] != '\0') {
            cmd[l] = temp->pName[i];
            i++;
            l++;
        }

        cmd[l] = '\0';

        if (WIFEXITED(stat)) {
            if (!fg_running && dur > 0)
                printf("\n");

            if ((WEXITSTATUS(stat)) == 0)
                printf("%s with pid = %d exited normally # After %ld seconds\n", cmd, bg_pid, dur);
            else
                printf("%s with pid = %d exited abnormally # After %ld seconds\n", cmd, bg_pid, dur);
        }
        else {
            if (!fg_running && dur > 0)
                printf("\n");

            printf("%s with pid = %d exited abnormally # After %ld seconds\n", temp->pName, bg_pid, dur);
        }

        if (!fg_running && dur > 0)
            printNames();

        delProcess(bg_pid);
        bg_count--;
        fflush(stdout);
    }
}

// Signal handler function to handle SIGINT signal (CTRL+C)
void end_fg_process(int sig) {
    printf("\n");

    if (fg_running)
        kill(fg_Proc.pid, SIGINT);

    if (!fg_running) {
        printNames();
        ctrl_c = true;
    }

    fflush(stdout);

    fg_running = false;
}

// Signal handler function to handle SIGTSTP signal (CTRL+Z)
void stop_fg_process(int sig) {
    bool p = true;
    int fg_pid = fg_Proc.pid;

    if (fg_pid != -1) {
        p = false;
        numProcess++;

        kill(fg_pid, SIGTTIN);
        kill(fg_pid, SIGTSTP);

        bg_count++;
        updatePlist(fg_pid, fg_Proc.pName);
    }

    printf("\n");

    if (p)
        printNames();

    fflush(stdout);
}
