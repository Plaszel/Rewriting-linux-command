
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>


void main(){
    int i = 3;
    char * znaki[] = {"ls","a","-b"};
    char * args;
    args = znaki[2] + 1;
    //memmove(znaki[2],znaki[2] + 1,strlen(znaki[2]));
    printf("%s\n",args);

}