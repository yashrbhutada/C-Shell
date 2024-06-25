#include "echo.h"
#include "prompt.h"

void echo(char *prompt, int i, int len) {
    // Removes the initial spaces
    while (i < len && (prompt[i] == ' ' || prompt[i] == '\t'))
        i++;

    // Prints the argument as it is by removing the spaces outside inverted commas and keeping the ones inside it
    while (i < len) {

        // If inverted comma is detected
        if (prompt[i] == '"') {
            i++;
            
            while (i < len && prompt[i] != '"') {
                printf("%c", prompt[i]);
                i++;
            }
            i++;

            continue;
        }

        // If space outside inverted comma is detected
        if (prompt[i] == ' ' || prompt[i] == '\t') {
            printf(" ");

            while (i < len && (prompt[i] == ' ' || prompt[i] == '\t'))
                i++;

            continue;
        }

        // If character is detected
        printf("%c", prompt[i]);
        i++;
    }

    printf("\n");
    return;
}
