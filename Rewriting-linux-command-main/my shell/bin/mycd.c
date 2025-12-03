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

    //changing dir
    if (chdir(path))
    {
        perror("Unknown location");
    }

    return 0;
}