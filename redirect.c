#include "redirect.h"
#include "prompt.h"

void check_redirection(char *prompt) {
    memset(inpFile, 0, 256);
    memset(outFile, 0, 256);
    memset(arg, 0, 256);

    iR = false;
    oR = false;
    to_append = false;
    
    int len = strlen(prompt);

    int p = 0;

    for (int i = 0; i < len; ++i) {

        // Input Redirection
        if (prompt[i] == '<') {
            i++;
            int j = 0;
            iR = true;

            // Ignoring extra spaces
            while (i < len && prompt[i] == ' ')
                i++;
            
            //Finding the file to take input from
            while (i < len && prompt[i] != ' ') {

                if (prompt[i] == '>' || prompt[i] == '<') {
                    i++;
                    break;
                }

                inpFile[j] = prompt[i];
                i++;
                j++;
            }

            if (i < len)
                i--;
        }

        //Output redirection
        else if (prompt[i] == '>') {
            i++;
            oR = true;

            if (i < len && prompt[i] == '>')
                to_append = true;

            i++;
            int j = 0;

            while (i < len && prompt[i] == ' ')
                i++;

            while (i < len && prompt[i] != ' ') {
                if (prompt[i] == '>' || prompt[i] == '<') {
                    i++;
                    break;
                }

                outFile[j] = prompt[i];
                i++;
                j++;
            }

            if (i < len)
                i--;
        }
        else {
            arg[p] = prompt[i];
            p++;
        }
    }
}

bool do_redirection() {
    if (iR) {
        int fd = open(inpFile, O_RDONLY);

        if (fd == -1) {

            if (strlen(inpFile) == 0)
                printf("bash: syntax error near unexpected token `newline'\n");
            else
                printf("shell: %s: No such file or directory\n", inpFile);

            return false;
        }

        dup2(fd, 0);
        close(fd);
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

            return false;
        }

        dup2(fd, 1);
        close(fd);
    }

    return true;
}
