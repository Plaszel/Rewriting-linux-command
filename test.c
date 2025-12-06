
    #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <math.h>

//dynamic memory appending for strings
void append(char ** str,char * add)
{
    int old_len = *str ? strlen(*str) : 0;
    int add_len = strlen(add);
    
    char *newptr = realloc(*str,old_len + add_len + 1);
    if (!newptr) exit(1);

    memcpy(newptr + old_len, add, add_len + 1);
    *str = newptr;
}

int main() {
   
    struct stat filestat;
   printf("%d", stat "test");
}

