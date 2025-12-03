#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 20

// parsing user input into command and arguments
void parseargument(char *cmd, char **args, int *count)
{
    int i = 0;
    char *temp = strtok(cmd, " ");
    while (temp && i < MAX_ARGS - 1)
    {
        args[i++] = temp;
        temp = strtok(NULL, " ");
    }
    *count = i;
    args[i] = NULL;
}

// triming spaces
void trimspace(char *str)
{
    // Remove leading spaces
    char *start = str;
    while (isspace((unsigned char)*start))
        start++;

    if (start != str)
    {
        memmove(str, start, strlen(start) + 1);
    }

    // Remove trailing spaces
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }
}

int main()
{
    char *pwd = getenv("PWD");
    setenv("PATH", pwd, 1);

    char command[100];
    int args_count;
    char *args[MAX_ARGS];

    while (true)
    {
        char *user = getenv("USER");
        char *workdir = getcwd(NULL, 0);

        size_t len = snprintf(NULL, 0, "[%s # %s] >", user, workdir) + 1;
        char *consoleinfo = malloc(len);

        // basic console output and reading user input
        sprintf(consoleinfo, "[%s # %s] >", user, workdir);
        printf("%s", consoleinfo);

        fgets(command, sizeof(command), stdin);

        // triming user input from \n spaces and parsing args
        command[strcspn(command, "\n")] = 0;
        trimspace(command);
        parseargument(command, args, &args_count);

        // exit shell
        if (!strcmp(args[0], "exit"))
        {
            free(consoleinfo);
            break;
        }

        // changing dir

        if (!strcmp(args[0], "cd"))
        {
            char *path;

            if (args_count > 2)
            {
                printf("Too many arguments give\n");
            }
            else
            {
                path =  (args_count > 1) ? args[1] : getenv("HOME");
                if (chdir(path))
                {
                    perror("cd");
                }
            }
            continue;
        }

        // executing external commands
        pid_t pid = fork();
        if (pid == 0)
        {
            execvp(args[0], args);
            perror("command failed");
            exit(1);
        }
        else
        {
            wait(NULL);
        }

        free(workdir);
        free(consoleinfo);
    }

    return 0;
}