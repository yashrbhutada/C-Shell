#include "foreground.h"
#include "redirect.h"
#include "prompt.h"
#include "echo.h"
#include "cd.h"
#include "ls.h"
#include "pastevents.h"
#include "seek.h"
#include "proclore.h"
#include "fgbg.h"
#include "jobs.h"
#include "pKill.h"

// Tokenize with pipes
command check_pipes(char *prompt) {
    char *token;
    char delim[] = {'|'};

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

// Execute commands without pipes
void execute(char *prompt) {
    check_redirection(prompt);

    if (!do_redirection())
        return;

    // Finding the first argument of the command to identify its type
    int len = strlen(arg);
    int i = 0;

    while (i < len && (arg[i] == ' ' || arg[i] == '\t'))
        i++;

    int l = 0;
    char cmd[MAX_ARG_LEN];
    while (arg[i] != ' ' && arg[i] != '\t' && arg[i] != '\0') {
        cmd[l] = arg[i];
        i++;
        l++;
    }

    cmd[l] = '\0';

    // Builtin commands
    if (strcmp(cmd, "cd") == 0) {
        cd(arg);
        return;
    }

    if (strcmp(cmd, "pwd") == 0) {
        printf("%s\n", curr_path);
        return;
    }

    if (strcmp(cmd, "echo") == 0) {
        echo(arg, i, len);
        return;
    }

    if (strcmp(cmd, "ls") == 0) {
        ls(arg);
        return;
    }

    if (strcmp(cmd, "proclore") == 0) {
        proclore(arg);
        return;
    }

    if (strcmp(cmd, "seek") == 0) {
        seek(arg);
        return;
    }

    if (strcmp(cmd, "pastevents") == 0) {
        printHistory();
        return;
    }

    if (strcmp(cmd, "exit") == 0) {
        exit(0);
    }

    if (strcmp(cmd, "jobs") == 0) {
        jobs(arg);
        return;
    }

    if (strcmp(cmd, "ping") == 0) {
        pKill(arg);
        return;
    }

    if (strcmp(cmd, "fg") == 0) {
        fg(arg);
        return;
    }

    if (strcmp(cmd, "bg") == 0) {
        bg(arg);
        return;
    }

    if (cmd[0] == '\0')
        return;

    // System commands
    fg_running = true;
    time_t seconds = time(NULL);
    command argv = getArgv(arg);

    int forkRet = fork();

    if (forkRet < 0) {
        printf("Error: Process fork failed\n");
        return;
    }
    else if (forkRet == 0) {
        setpgrp();

        int x = execvp(argv.str_vec[0], argv.str_vec);

        if (x == -1) {
            printf("%s: Command not found\n", argv.str_vec[0]);
            exit(1);
        }

        exit(0);
    }
    else {
        fg_Proc.job_Num = 0;
        fg_Proc.pid = forkRet;
        strcpy(fg_Proc.pName, argv.str_vec[0]);

        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(0, forkRet);

        int status = 0;
        waitpid(forkRet, &status, WUNTRACED);

        tcsetpgrp(0, getpgrp());
        signal(SIGTTOU, SIG_DFL);

        if (WIFSTOPPED(status)) {
            bg_count++;
            numProcess++;

            printf("\n");
            updatePlist(forkRet, arg);
        }

        if (WIFSIGNALED(status))
            printf("\n");

        fg_running = false;
        fg_Proc.pid = -1;

        seconds = time(NULL) - seconds;

        if (seconds > 1)
            printf("# %s for %ld seconds\n", argv.str_vec[0], seconds);
    }

    return;
}

// Execute commands with pipes
void execute_pipes(char *prompt) {
    // Findind the first argument of the command to identify its type
    int len = strlen(arg);
    int i = 0;

    while (i < len && (arg[i] == ' ' || arg[i] == '\t'))
        i++;

    int l = 0;
    char cmd[MAX_ARG_LEN];
    while (arg[i] != ' ' && arg[i] != '\t' && arg[i] != '\0') {
        cmd[l] = arg[i];
        i++;
        l++;
    }

    cmd[l] = '\0';

    // Builtin commands
    if (strcmp(cmd, "cd") == 0) {
        cd(arg);
        return;
    }

    if (strcmp(cmd, "pwd") == 0) {
        printf("%s\n", curr_path);
        return;
    }

    if (strcmp(cmd, "echo") == 0) {
        echo(arg, i, len);
        return;
    }

    if (strcmp(cmd, "ls") == 0) {
        ls(arg);
        return;
    }

    if (strcmp(cmd, "proclore") == 0) {
        proclore(arg);
        return;
    }

    if (strcmp(cmd, "seek") == 0) {
        seek(arg);
        return;
    }

    if (strcmp(cmd, "pastevents") == 0) {
        printHistory();
        return;
    }

    if (strcmp(cmd, "exit") == 0)
        exit(0);

    if (strcmp(cmd, "jobs") == 0) {
        jobs(arg);
        return;
    }

    if (strcmp(cmd, "ping") == 0) {
        pKill(arg);
        return;
    }

    if (strcmp(cmd, "fg") == 0) {
        fg(arg);
        return;
    }

    if (strcmp(cmd, "bg") == 0) {
        bg(arg);
        return;
    }

    if (cmd[0] == '\0')
        return;

    // System commands
    fg_running = true;
    time_t seconds = time(NULL);
    command argv = getArgv(arg);

    int forkRet = fork();

    if (forkRet < 0) {
        printf("Error: Process fork failed\n");
        return;
    }
    else if (forkRet == 0) {
        setpgrp();

        int x = execvp(argv.str_vec[0], argv.str_vec);

        if (x == -1) {
            printf("%s: Command not found\n", argv.str_vec[0]);
            exit(1);
        }

        exit(0);
    }
    else {
        fg_Proc.job_Num = 0;
        fg_Proc.pid = forkRet;
        strcpy(fg_Proc.pName, argv.str_vec[0]);

        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(0, forkRet);

        int status = 0;
        waitpid(forkRet, &status, WUNTRACED);

        tcsetpgrp(0, getpgrp());
        signal(SIGTTOU, SIG_DFL);

        if (WIFSTOPPED(status)) {
            bg_count++;
            numProcess++;

            printf("\n");
            updatePlist(forkRet, arg);
        }

        if (WIFSIGNALED(status))
            printf("\n");

        fg_running = false;
        fg_Proc.pid = -1;

        seconds = time(NULL) - seconds;

        if (seconds > 1)
            printf("# %s for %ld seconds\n", argv.str_vec[0], seconds);
    }

    return;
}

// Detect for pipes and execute accordingly
void foreground(char *prompt) {
    command pipes = check_pipes(prompt);

    if (pipes.num == 1)
        execute(prompt);
    else {
        int p[pipes.num][2];

        if (pipe(p[0]) == -1) {
            printf("shell: pipe creation failed\n");
            return;
        }

        check_redirection(pipes.str_vec[0]);

        if (oR) {
            printf("shell: Output buffer identified in between\n");
            return;
        }

        if (iR) {
            int fd = open(inpFile, O_RDONLY);

            if (fd == -1) {

                if (strlen(inpFile) == 0)
                    printf("shell: syntax error near unexpected token `newline'\n");
                else
                    printf("shell: %s: No such file or directory\n", inpFile);

                return;
            }

            dup2(fd, 0);
            close(fd);
        }

        dup2(p[0][1], 1);
        close(p[0][1]);

        int f_in = p[0][0];
        int f_out = p[0][1];

        execute_pipes(pipes.str_vec[0]);

        for (int i = 1; i < pipes.num - 1; ++i) {
            pipe(p[i]);

            check_redirection(pipes.str_vec[i]);

            if (iR || oR) {
                // printf("shell: Output or Input buffer identified in between\n");
                write(stdout_save, "shell: Output or Input buffer identified in between\n", 53);
                return;
            }

            dup2(f_in, 0);
            dup2(p[i][1], 1);

            execute_pipes(pipes.str_vec[i]);

            close(p[i][1]);
            f_in = p[i][0];
            f_out = p[i][1];
        }

        check_redirection(pipes.str_vec[pipes.num - 1]);

        if (iR) {
            // printf("shell: Output buffer identified in between\n");
            write(stdout_save, "shell: Input buffer identified in between\n", 43);

            return;
        }

        if (oR) {
            int fd;

            if (to_append)
                fd = open(outFile, O_RDWR | O_CREAT | O_APPEND, 0644);
            else
                fd = open(outFile, O_RDWR | O_CREAT | O_TRUNC, 0644);

            if (fd == -1) {
                if (strlen(inpFile) == 0)
                    printf("bash: syntax error near unexpected token `newline'\n");
                else
                    printf("shell: %s: No such file or directory\n", inpFile);

                return;
            }

            dup2(fd, 1);
            close(fd);
        }
        else {
            dup2(stdout_save, STDOUT_FILENO);
            close(stdout_save);
        }

        dup2(f_in, 0);
        execute_pipes(pipes.str_vec[pipes.num - 1]);

        for (int i = 0; i < pipes.num; ++i) {
            close(p[i][0]);
            close(p[i][1]);
        }
    }

    return;
}
