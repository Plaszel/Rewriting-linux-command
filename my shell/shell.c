#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void trimspace(char *str) {
    // Remove leading spaces
    char *start = str;
    while (isspace((unsigned char)*start)) start++;

    if (start != str) {
        memmove(str, start, strlen(start) + 1); 
    }

    // Remove trailing spaces
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

int main()
    {

        while (true)
            {
                char * user = getenv("USER");
                char * workdir = getenv("PWD");

                size_t len = snprintf(NULL, 0, "[%s # %s] >",user ,workdir) + 1;
                char * consoleinfo = malloc(len);

                //basic console output and reading user input
                sprintf(consoleinfo,"[%s # %s] >",user,workdir);
                printf("%s",consoleinfo);

                char command[100];
                fgets(command, sizeof(command), stdin);

                // checking for exit
                command[strcspn(command,"\n")] = 0;
                trimspace(command);
                if (!strcmp(command, "exit"))
                    {
                        break;
                    }

                free(consoleinfo);
            }

            return 0;
    }