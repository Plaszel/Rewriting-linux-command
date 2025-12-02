#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 20

// parsing user input into command and arguments
void parse(char *cmd, char **args)
    {
        int i = 0;
        char * temp = strtok(cmd," ");
        while (temp && i < MAX_ARGS -1)
        {
            args[i++] = temp;
            temp = strtok(NULL, " ");
        }
        args[i] = NULL;
    }

//triming spaces
void trimspace(char *str) 
    {
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
        char*  pwd  = getenv("PWD");
        setenv("PATH", pwd , 1);

        char command[100];
        char *args[MAX_ARGS];
        
        while (true)
            {
                char * user = getenv("USER");
                char * workdir = getenv("PWD");

                size_t len = snprintf(NULL, 0, "[%s # %s] >",user ,workdir) + 1;
                char * consoleinfo = malloc(len);

                //basic console output and reading user input
                sprintf(consoleinfo,"[%s # %s] >",user,workdir);
                printf("%s",consoleinfo);

                fgets(command, sizeof(command), stdin);

                // checking for exit
                command[strcspn(command,"\n")] = 0;
                trimspace(command);
                if (!strcmp(command, "exit"))
                    {
                        break;
                    }
                
                //executing commands
                parse(command, args);
                pid_t pid = fork();
                if (pid == 0) 
                    {
                        execvp(args[0], args);
                        perror("execvp failed");
                        exit(1);
                    } else 
                        {
                            wait(NULL);
                        }
                
                free(consoleinfo);
            }

            return 0;
    }