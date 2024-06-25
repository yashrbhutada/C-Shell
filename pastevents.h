#ifndef __PASTEVENTS_H
#define __PASTEVENTS_H

#define MAX_HIS_NUM 20
#define MAX_ARG_LEN 256

char nline[2];

char histPath[256];
char history[MAX_HIS_NUM][MAX_ARG_LEN];
int numHistory;
int nextPos;
int startPos;

void openHistoryFile();
void updateHistory(char* prompt);
void updateHistoryFile();
void printHistory();

#endif
