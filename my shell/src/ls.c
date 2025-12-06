#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
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

//smt for qsort 
int compareDirent(const void *a, const void *b) 
{
    struct dirent *entryA = *(struct dirent **)a;
    struct dirent *entryB = *(struct dirent **)b;
    return strcmp(entryA->d_name, entryB->d_name);
}


int main(int argc, char *argv[])
{

    //getting user input for paths and args
    char ** paths = NULL;
    char * args = NULL;
    int index_args = 0;
    int index_paths = 0;
    
    for (int i = 1;i < argc;i++){
        if (argv[i][0] == '-')
        {
            append(&args, argv[i] + 1);
        }else{
            paths = realloc(paths,index_paths * sizeof(*paths));
            paths[index_paths++] = argv[i]; 
        }
    }

    
    if (!index_paths)
    {
        paths = realloc(paths,index_paths * sizeof(*paths));
        paths[0] = getcwd(NULL, 0);
        index_paths++;
    }
    

    bool show_all = (args && strchr(args,'a')) ? true : false;
    bool list = (args && strchr(args,'l')) ? true : false;

    for (int i = 0;i < index_paths;i++) 
    {
        
        // opening dir and checking if it exists
        DIR *d = opendir(paths[i]);
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
        if (index_paths > 1)
            printf("\033[34m" "%s:\n" "\033[0m",paths[i]);

        if(list)
        {
            int size_long = 0;

            for (int j = 0; j < count; j++)
            {
                //align buffer for size printing
                    struct stat filestat;
                    char full_path[256];
                    snprintf(full_path,sizeof(full_path),"%s%s%s",paths[i],"/",directories[j]->d_name);
                    stat(full_path,&filestat);
                    int temp_size_long = 0;

                    while (true)
                    {
                        if(filestat.st_size < pow(10, temp_size_long))
                            break; 
                        temp_size_long++;
                    }

                    if (temp_size_long > size_long)
                        size_long = temp_size_long;
            }

            for (int j = 0; j < count; j++)
            {
                struct stat filestat;
                char full_path[256];
                snprintf(full_path,sizeof(full_path),"%s%s%s",paths[i],"/",directories[j]->d_name);
                stat(full_path,&filestat);
                char * line_output = NULL;
                static mode_t permppl[] = {S_IRWXO, S_IRWXG, S_IRWXU};

                //show hidde?
                if ( *directories[j]->d_name == '.' && !show_all)
                    continue; 

                //getting type of file
                if (S_ISREG(filestat.st_mode)) append(&line_output, "-");
                if (S_ISDIR(filestat.st_mode)) append(&line_output, "d");
                if (S_ISLNK(filestat.st_mode)) append(&line_output, "s");
                if (S_ISCHR(filestat.st_mode)) append(&line_output, "c");
                if (S_ISBLK(filestat.st_mode)) append(&line_output, "b");
                if (S_ISFIFO(filestat.st_mode)) append(&line_output, "p");
                if (S_ISSOCK(filestat.st_mode)) append(&line_output, "s");                                                 

                // getting file perms
                for (int j = 2; j >= 0; j--)
                {
                int temp = (filestat.st_mode & permppl[j]) >> 3*j;

                    char * read = (temp & 0b100) ? "r" : "-";
                    char * write = (temp & 0b010) ? "w" : "-";
                    char * execute = (temp & 0b001) ? "x" : "-";
                    append(&line_output, read);
                    append(&line_output, write);
                    append(&line_output, execute);
                
                }

                //getting user and group id
                uid_t uid = filestat.st_uid;
                gid_t gid = filestat.st_gid;

                //covert ids to names
                struct passwd *usr = getpwuid(uid);
                if (usr == NULL)
                {
                    perror("getpwuid");
                    return 1;
                }

                struct group *grp = getgrgid(gid);
                if (grp == NULL)
                {
                    perror("getgrgid");
                    return 1;
                }

                //date formating 
                char timebuf[64];

                strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", localtime(&filestat.st_mtime));

                
                //output 
                printf("%s  %s  %s  %*lld  %s  %s\n",
                    line_output,
                    usr->pw_name,
                    grp->gr_name,
                    size_long ,(long long)filestat.st_size, 
                    timebuf,
                    directories[j]->d_name);
                free(line_output);
            }
        }else{

            for (int j = 0; j < count; j++)
            {
                if ( *directories[j]->d_name == '.' && !show_all)
                    continue;
                printf("%s  ",directories[j]->d_name);
            }
            printf("\n");
             
        }

        //clearing memory
        for (int j = 0; j < count; j++)
            free(directories[j]);
        free(directories);

        
        closedir(d);
        if (i != index_paths-1)
            printf("\n"); 
        
    }
    free(paths);

    return 0;
}