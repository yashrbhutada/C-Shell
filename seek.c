#include "seek.h"
#include "prompt.h"

void seek(char *prompt) {
    command cmd = getArgv(prompt);

    bool dir_given = false;
    bool d_flag = false;
    bool f_flag = false;
    bool name_given = false;

    // Parsing and checking what type of arguments are available
    for (int i = 1; i < cmd.num; ++i) {
        if (cmd.str_vec[i][0] == '-') {
            if (cmd.str_vec[i][1] == 'd')
                d_flag = true;
            else if (cmd.str_vec[i][1] == 'f')
                f_flag = true;
            else {
                printf("seek: unrecognized option '%s'\n", cmd.str_vec[i]);
                return;
            }
        }
        else if (cmd.str_vec[i][0] == '"') {
            name_given = true;

            int len = strlen(cmd.str_vec[i]);
            for (int j = 0; j < len - 1; ++j)
                cmd.str_vec[i][j] = cmd.str_vec[i][j + 1];

            cmd.str_vec[i][len - 2] = '\0';
        }
        else
            dir_given = true;
    }

    char dir_path[PATH_MAX];

    // Getting the parent directory from where search will start
    if (dir_given)
        strcpy(dir_path, cmd.str_vec[1]);
    else
        strcpy(dir_path, ".");

    // Discovering files according to the arguments given
    if (!name_given)
        printDiscoverAll(dir_path, d_flag, f_flag, 1);
    else
        printDiscoverName(dir_path, cmd.str_vec[cmd.num - 1], d_flag, f_flag);
}

// Recursively prints corresponding files and directories if name is not given
void printDiscoverAll(char *dir_path, bool d_flag, bool f_flag, int main_func) {
    bool pDir = (!(d_flag ^ f_flag)) | d_flag;
    bool pFile = (!(d_flag ^ f_flag)) | f_flag;

    DIR *directory = opendir(dir_path);

    if (directory == NULL) {
        printf("seek: cannot access '%s': No such directory\n", dir_path);
        return;
    }

    struct dirent *dir;

    // Print name of parent directory
    if (pDir && main_func)
        printf("%s\n", dir_path);

    // Print names of all files and directories according to the flags
    while ((dir = readdir(directory)) != NULL) {
        if (dir->d_name[0] == '.')
            continue;

        if (dir->d_type == DT_DIR) {
            char new_path[PATH_MAX*2];
            strcpy(new_path, dir_path);
            strcat(new_path, "/");
            strcat(new_path, dir->d_name);

            if (pDir)
                printf(BLUE "%s\n", new_path);

            printDiscoverAll(new_path, d_flag, f_flag, 0);
        }
        else {
            if (pFile) {
                char new_path[PATH_MAX*2];
                strcpy(new_path, dir_path);
                strcat(new_path, "/");
                strcat(new_path, dir->d_name);

                printf(GREEN "%s\n", new_path);
            }
        }
    }
}

// Recursively prints corresponding files and directories if name is given
void printDiscoverName(char *dir_path, char *file_name, bool d_flag, bool f_flag) {
    bool pDir = (!(d_flag ^ f_flag)) | d_flag;
    bool pFile = (!(d_flag ^ f_flag)) | f_flag;

    // Opening the parent directory
    DIR *directory = opendir(dir_path);

    if (directory == NULL) {
        printf("seek: cannot access '%s': No such directory\n", dir_path);
        return;
    }

    struct dirent *dir;

    // Print names of all files and directories according to the flags and name
    while ((dir = readdir(directory)) != NULL) {
        if (dir->d_name[0] == '.')
            continue;

        if (dir->d_type == DT_DIR) {
            char new_path[PATH_MAX];
            strcpy(new_path, dir_path);
            strcat(new_path, "/");
            strcat(new_path, dir->d_name);

            if (pDir && strcmp(dir->d_name, file_name) == 0)
                printf("%s\n", new_path);

            printDiscoverName(new_path, file_name, d_flag, f_flag);
        }
        else {
            if (pFile && strcmp(dir->d_name, file_name) == 0) {
                char new_path[PATH_MAX];
                strcpy(new_path, dir_path);
                strcat(new_path, "/");
                strcat(new_path, dir->d_name);

                printf("%s\n", new_path);
            }
        }
    }
}
