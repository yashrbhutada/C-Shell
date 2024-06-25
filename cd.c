#include "cd.h"
#include "prompt.h"

void cd(char *prompt) {
    // Get the path to the new directory
    command cmd = getArgv(prompt);

    // Checking number of arguments
    if (cmd.num > 2) {
        printf("shell: cd: too many arguments\n");
        return;
    }

    bool success = true;
    path_has_home = true;

    // Changing directory based on different situations
    if (cmd.num == 1)
        chdir(home_path);
    else if (strcmp(cmd.str_vec[1], "~") == 0)
        chdir(home_path);
    else if (strcmp(cmd.str_vec[1], "-") == 0) {
        printf("%s\n", prev_path);
        chdir(prev_path);
    }
    else {
        if (chdir(cmd.str_vec[1]) == -1) {
            if (errno == ENOENT) {
                success = false;
                printf("shell: cd: %s: No such file or directory\n", cmd.str_vec[1]);
            }
        }
    }

    // Getting new directory path and updating our global variables
    if (success) {
        strcpy(prev_path, curr_path);
        getcwd(curr_path, PATH_MAX);

        // Checking if new directory path contains home or not
        int len_home = strlen(home_path);
        for (int i = 0; i < len_home; ++i) {
            if (curr_path[i] != home_path[i]) {
                path_has_home = false;
                break;
            }
        }

        // Modifying the prompt shown according to the new path
        if (path_has_home) {
            mod_path[0] = '~';
            strcpy(mod_path + 1, curr_path + len_home);
        }
        else
            strcpy(mod_path, curr_path);

    }
}
