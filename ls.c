#include "prompt.h"
#include "ls.h"

// Compare function for two strings
static int myCompare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Compare function for alphasort
extern int alphasort(const struct dirent **__e1, const struct dirent **__e2) {
    return strcasecmp((*__e1)->d_name, (*__e2)->d_name);
}

//Function to check the flags to list
void ls(char *prompt) {
    command cmd = getArgv(prompt);

    bool list_all = false;
    bool info_all = false;
    bool path_exist = false;

    
    int i = 1;

    // Parses the arguments and identifies the flags and directories
    for (; i < cmd.num; i++) {
        if (cmd.str_vec[i][0] == '-') {
            int k = 1;
            while (cmd.str_vec[i][k] != '\0') {
                if (cmd.str_vec[i][k] == 'a')
                    list_all = true;
                else if (cmd.str_vec[i][k] == 'l')
                    info_all = true;
                else {
                    printf("ls: unrecognized option '%s'\n", cmd.str_vec[i]);
                    return;
                }

                k++;
            }
        }
        else {
            path_exist = true;
            break;
        }
    }

    // Prints accordingly to the given flags and directories
    if (!path_exist)
        printDir(curr_path, list_all, info_all, false);
    else if (i == cmd.num - 1)
        printDir(cmd.str_vec[i], list_all, info_all, false);
    else {
        for (; i < cmd.num; ++i) {
            printDir(cmd.str_vec[i], list_all, info_all, true);

            if (i != cmd.num - 1)
                printf("\n");
        }
    }
}

// Function to print the contents of the directory
void printDir(char *path, bool list_all, bool info_all, bool print_name) {
    struct dirent **namelist;

    // Scans and sorts the contents of the directory alphabetically
    int num_entries = scandir(path, &namelist, 0, alphasort);

    if (num_entries == -1) {
        if (errno == ENOENT) {
            printf("ls: cannot access '%s': No such file or directory\n", path);
            return;
        }

        // If the given name is not a directory
        if (errno == ENOTDIR) {
            struct stat info;
            lstat(path, &info);

            if (!info_all) {
                if (info.st_mode & S_IXUSR)
                    printf(RED "%s\n" RESET, path);
                else
                    printf("%s\n", path);
            }
            else {
                // Identifying file type
                if (info.st_mode & S_IFDIR)
                    printf("%c", 'd');
                else if (info.st_mode & S_IFCHR)
                    printf("%c", 'c');
                else if (info.st_mode & S_IFBLK)
                    printf("%c", 'b');
                else if (info.st_mode & S_IFREG)
                    printf("%c", '-');
                else if (info.st_mode & S_IFLNK)
                    printf("%c", 'l');
                else if (info.st_mode & S_IFSOCK)
                    printf("%c", 's');
                else if (info.st_mode & S_IFIFO)
                    printf("%c", 'f');

                printf("%c", info.st_mode & S_IRUSR ? 'r' : '-');
                printf("%c", info.st_mode & S_IWUSR ? 'w' : '-');
                printf("%c", info.st_mode & S_IXUSR ? 'x' : '-');
                printf("%c", info.st_mode & S_IRGRP ? 'r' : '-');
                printf("%c", info.st_mode & S_IWGRP ? 'w' : '-');
                printf("%c", info.st_mode & S_IXGRP ? 'x' : '-');
                printf("%c", info.st_mode & S_IROTH ? 'r' : '-');
                printf("%c", info.st_mode & S_IWOTH ? 'w' : '-');
                printf("%c", info.st_mode & S_IXOTH ? 'x' : '-');
                printf(" ");

                printf("%ld", info.st_nlink);
                printf(" ");

                struct passwd *user = getpwuid(info.st_uid);
                struct group *grp = getgrgid(info.st_gid);

                printf("%4s", user->pw_name);
                printf(" ");

                printf("%4s", grp->gr_name);
                printf(" ");

                printf("%6.ld", info.st_size);
                printf(" ");

                command gmt_time = getArgv(ctime(&info.st_mtime));
                printf("%s %s %.5s", gmt_time.str_vec[1], gmt_time.str_vec[2], gmt_time.str_vec[3]);
                printf(" ");

                if (info.st_mode & S_IXUSR)
                    printf(RED "%s\n" RESET, path);
                else
                    printf("%s\n", path);
            }

            return;
        }
    }

    // If directory is given
    if (print_name)
        printf("%s:\n", path);

    struct stat info[num_entries];
    long long total = 0;

    int i = 0;
    
    // Calculating total blocks occupied by the directory
    while (i < num_entries) {
        char new_path[PATH_MAX];
        strcpy(new_path, path);
        strcat(new_path, "/");
        strcat(new_path, namelist[i]->d_name);

        lstat(new_path, &info[i]);

        if (!list_all && namelist[i]->d_name[0] == '.') {
            i++;
            continue;
        }

        total += info[i].st_blocks;
        i++;
    }

    if (info_all)
        printf("total %lld\n", total / 2);

    i = 0;

    // Printing the information of each file and folder in the directory
    while (i < num_entries) {
        if (!list_all && namelist[i]->d_name[0] == '.') {
            i++;
            continue;
        }
        else {
            if (info_all) {
                // Identifying file type
                if (info[i].st_mode & S_IFDIR)
                    printf("%c", 'd');
                else if (info[i].st_mode & S_IFCHR)
                    printf("%c", 'c');
                else if (info[i].st_mode & S_IFBLK)
                    printf("%c", 'b');
                else if (info[i].st_mode & S_IFREG)
                    printf("%c", '-');
                else if (info[i].st_mode & S_IFLNK)
                    printf("%c", 'l');
                else if (info[i].st_mode & S_IFSOCK)
                    printf("%c", 's');
                else if (info[i].st_mode & S_IFIFO)
                    printf("%c", 'f');

                printf("%c", info[i].st_mode & S_IRUSR ? 'r' : '-');
                printf("%c", info[i].st_mode & S_IWUSR ? 'w' : '-');
                printf("%c", info[i].st_mode & S_IXUSR ? 'x' : '-');
                printf("%c", info[i].st_mode & S_IRGRP ? 'r' : '-');
                printf("%c", info[i].st_mode & S_IWGRP ? 'w' : '-');
                printf("%c", info[i].st_mode & S_IXGRP ? 'x' : '-');
                printf("%c", info[i].st_mode & S_IROTH ? 'r' : '-');
                printf("%c", info[i].st_mode & S_IWOTH ? 'w' : '-');
                printf("%c", info[i].st_mode & S_IXOTH ? 'x' : '-');
                printf(" ");

                printf("%3ld", info[i].st_nlink);
                printf(" ");

                struct passwd *user = getpwuid(info[i].st_uid);
                struct group *grp = getgrgid(info[i].st_gid);

                printf("%4s", user->pw_name);
                printf(" ");

                printf("%4s", grp->gr_name);
                printf(" ");

                printf("%6.ld", info[i].st_size);
                printf(" ");

                command gmt_time = getArgv(ctime(&info[i].st_mtime));
                printf("%s %s %.5s", gmt_time.str_vec[1], gmt_time.str_vec[2], gmt_time.str_vec[3]);
                printf(" ");
            }

            if (namelist[i]->d_type == DT_DIR)
                printf(BLUE "%s\n" RESET, namelist[i]->d_name);
            else if (info[i].st_mode & S_IXUSR)
                printf(RED "%s\n" RESET, namelist[i]->d_name);
            else
                printf("%s\n", namelist[i]->d_name);

            free(namelist[i]);
            i++;
        }
    }

    free(namelist);
}
