#ifndef __SEEK_H
#define __SEEK_H

typedef int bool;

void seek(char* prompt);
void printDiscoverAll(char* dir_path, bool d_flag, bool f_flag, int main_func);
void printDiscoverName(char *dir_path, char *file_name, bool d_flag, bool f_flag);

#endif