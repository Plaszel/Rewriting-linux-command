#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>


int compareDirent(const void *a, const void *b) {
    struct dirent *entryA = *(struct dirent **)a;
    struct dirent *entryB = *(struct dirent **)b;
    return strcmp(entryA->d_name, entryB->d_name);
}


int main(int argc, char *argv[]){

    //getting user input for paths and args
    char ** paths = NULL;
    char ** args = NULL;
    for (int i = 1;i < argc;i++){
        if (strchr(argv[i],*"-"))
        {
            int index = sizeof(args)/sizeof(args[0]);
            args = realloc(args,index + sizeof(*args));
            args[index-1] = malloc(index);
            args[index-1] = argv[i] + 1; 
        }else{
            int index = sizeof(paths)/sizeof(paths[0]);
            paths = realloc(paths,index + sizeof(*paths));
            paths[index-1] = malloc(index);
            paths[index-1] = argv[i]; 
        }
    }
    
    // TODO change output based on given args
    // add loop for few dirs

    // opening dir and checking if it exists
    DIR *d = opendir(paths[0]);
    if(!d){
        perror("ls");
        return 1;
    }

    // iterating each file and adding to array
    struct dirent *entry;
    struct dirent **directories = NULL;
    int count = 0;

    while ((entry = readdir(d)))
    {
        directories = realloc(directories, (count +1) * sizeof(struct dirent*));    
        directories[count] = malloc(sizeof(struct dirent));
        *directories[count] = *entry;
        count++;
    }
    
    //sorting alphabeticly
    qsort(directories, count ,sizeof(struct dirent *),compareDirent);


    // simple file displaying
    for (int i = 0; i < count; i++)
    {
        if ( *directories[i]->d_name == '.')
            continue;
        printf("%s  ",directories[i]->d_name);
    }
    printf("\n");
    

    for (int i = 0; i < count; i++)
        free(directories[i]);



    free(paths);
    free(args);
    free(directories);
    closedir(d);
    return 0;
}