#include "pastevents.h"
#include "prompt.h"

// Function to get history of previous sessions
void openHistoryFile() {
    // Constructing the path to the history file in home folder
    getcwd(histPath, PATH_MAX);
    strcat(histPath, "/");
    strcat(histPath, "history.txt");

    int hist_fd = open(histPath, O_RDONLY);

    if (hist_fd == -1)
    {
        printf("shell: error in getting history of previous sessions\n");
        return;
    }

    // Reading the history file into buffer
    int buf_size = 10240;
    char *buffer = (char *)calloc(buf_size, sizeof(char));

    read(hist_fd, buffer, 10240);

    int i = 0;
    char *token;

    token = strtok(buffer, nline);

    // Storing the history in a string array
    while (token)
    {
        strcpy(history[i], token);
        token = strtok(0, nline);
        i++;
    }

    close(hist_fd);

    numHistory = i;
    nextPos = i % MAX_HIS_NUM;
    startPos = numHistory > 10 ? numHistory - 10 : 0;
}

// Function to update the history array and file
void updateHistory(char *prompt) {
    int lastPos = nextPos > 0 ? nextPos - 1 : 19;

    if (numHistory > 0 && strcmp(history[lastPos], prompt) == 0)
        return;

    strcpy(history[nextPos], prompt);
    nextPos = (nextPos + 1) % MAX_HIS_NUM;
    startPos = (startPos + 1) % MAX_HIS_NUM;

    if (numHistory < MAX_HIS_NUM) {
        int hist_fd = open(histPath, O_RDWR | O_CREAT, 0600);

        lseek(hist_fd, 0, SEEK_END);
        write(hist_fd, prompt, strlen(prompt));
        write(hist_fd, nline, 2);

        close(hist_fd);

        numHistory++;
    }
    else
        updateHistoryFile();
}

// Function to update the history file
void updateHistoryFile() {
    int hist_fd = open(histPath, O_RDWR | O_CREAT | O_TRUNC);

    if (hist_fd == 1) {
        printf("shell: error in storing history of previous sessions\n");
        return;
    }

    for (int i = 0; i < MAX_HIS_NUM; ++i) {
        int len = strlen(history[(nextPos + i) % MAX_HIS_NUM]);
        write(hist_fd, history[(nextPos + i) % MAX_HIS_NUM], len);
        write(hist_fd, nline, 2);
    }

    close(hist_fd);
}

// Function to print the last 10 history
void printHistory() {
    int n = numHistory > 10 ? 10 : numHistory;
    for (int i = 0; i < n; ++i) {
        int idx = (startPos + i) % MAX_HIS_NUM;
        printf("%s\n", history[idx]);
    }
}
