#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char * path;
    //checking for validity of arguments
    if(argc > 1)
    {
        path = argv[1];
    }else if(argc > 2)
        {
            printf("\n%s","too many arguments given");
        }else
            {
                path = getenv("HOME");
            }
    if (chdir(path))
    {
        perror("Unknown location");
    }

    // get user shell by ppid
    __pid_t ppid = getppid();
    char path_to_proc[255];
    snprintf(path_to_proc,sizeof(path_to_proc),"/proc/%d/comm",ppid);

    FILE *f = fopen(path_to_proc,"r");
    if (!f) {
        perror("comm");
        return 1;
    }

    char shell[63];
    fgets(shell,sizeof(shell),f);

    // remove \n build shell path
    char path_to_shell[128];
    shell[strcspn(shell,"\n")] = 0;
    snprintf(path_to_shell, sizeof(path_to_shell), "/bin/%s", shell);

    fclose(f);

    //replace current shell with this process shell and ignores neofetch
    setenv("neofetch_ignore", "1", 1);
    execl(path_to_shell,shell,NULL);
    perror("execl");

    return 0;
}