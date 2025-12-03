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
    //getting user input for path
    char * path;
    if (argc > 1){
        path = argv[1];
    } else{
        path = ".";
    }
    

    // opening dir and checking if it exists
    DIR *d = opendir(path);
    if(!d){
        perror("dir");
        return 1;
    }

    

    struct dirent *entry;
    // iterating each file and adding to array
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
    free(directories);
    closedir(d);
    return 0;
}