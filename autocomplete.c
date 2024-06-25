#include "autocomplete.h"
#include "prompt.h"

// Function to autocomplete
int autocomplete(char *prompt) {
    int p = strlen(prompt);

    DIR *directory = opendir(".");

    if (directory == NULL) {
        printf("\nshell: error in opening current directory\n");
        return -1;
    }

    struct dirent *dir;
    int num_file = 0;

    char files[100][256];
    while ((dir = readdir(directory)) != NULL) {
        if (dir->d_name[0] == '.') continue;

        if (strncmp(prompt, dir->d_name, p) == 0) {
            strcpy(files[num_file], dir->d_name);

            if (dir->d_type == DT_DIR) {
                int len = strlen(files[num_file]);
                files[num_file][len] = '/';
                files[num_file][len + 1] = '\0';
            }

            num_file++;
        }
    }

    // If only a single file found, return that
    if (num_file == 1) {
        strcpy(prompt, files[0]);
    }

    //If multiple files found, display all files
    else {
        int min_len = INT_MAX;
        
        printf("\n");
        for (int i = 0; i < num_file; ++i) {
            min_len = (strlen(files[i]) < min_len) ? strlen(files[i]) : min_len;
            printf("%s\n", files[i]);
        }

        if (min_len == INT_MAX) return num_file;

        for (int i = p; i < min_len; ++i) {
            bool br = false;

            for (int j = 1; j < num_file; ++j) {
                if (files[j][i] != files[j-1][i]) {
                    br = true;
                    break;
                }
            }

            if (br) break;

            prompt[i] = files[0][i];
        }
    }

    return num_file;
}
