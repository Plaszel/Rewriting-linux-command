#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <stdbool.h>


struct termios termosik_stary;

struct output
{
    char * text;
    int lenght;
};

struct cursor
{
    int x;
    int y;
};

char readKey();

void appendChar(struct output * temp,char toadd, int pos);

void removeChar(struct output * str,struct cursor * cur,int pos);

int get_cursor_position(struct output * str,struct cursor * cur);

int get_max_rows(struct output *str);

int get_end_of_line(struct output * str,struct cursor * cur);

void move_cursor(struct cursor * temp, int x, int y);

void disable_termosik();

void enable_termosik();


//read key input 
char readKey(){
    char input[3];
    read(STDIN_FILENO, &input, 3); 
    if(input[0] == 27)
    {
        if (input[1] == 91)
        {  
            return input[2];
        }
        
    }else{
        return input[0];
    }
}

// adding single char to output
void appendChar( struct output * temp,char toadd, int pos)
{
        char * buff = realloc(temp->text,temp->lenght + 2);
        if (!buff) return;
        temp->text = buff;
        memmove(&temp->text[pos+1],&temp->text[pos],temp->lenght - pos);
        temp->text[pos] = toadd;
        temp->lenght++;
        if (toadd == '\r')
        {
            temp->lenght++;
            memmove(&temp->text[pos+1],&temp->text[pos],temp->lenght - pos);
            temp->text[pos+1] = '\n';
        }
        temp->text[temp->lenght] = '\0';
}

//removing char at position
void removeChar(struct output * str,struct cursor * cur,int pos)
{   
        if (cur->x == 1 && cur->y == 1)
        {
            return;
        }

        
        if (cur->x == 1)
        {
            cur->y--;
            if ( cur->y == 1)
            {
                cur->x = pos;
            }else{
                int i = 1;
                while( str->lenght > 0 && str->text[pos-1-i] != '\n')
                {
                    i++;
                }
                cur->x = i;
            }
        }else{
            cur->x--;
        }
        
        memmove(&str->text[pos],&str->text[pos+1],str->lenght - pos);        
        str->lenght--;
        str->text = realloc(str->text,str->lenght + 1);
        str->text[str->lenght] = '\0';
}

//get cursor position (lenght)
int get_cursor_position(struct output * str,struct cursor * cur)
{
    if (!str->text || str->lenght == 0) return 0;

    int row = 1;
    int col = 1;
    int i = 0;

    while (i < str->lenght)
    {
        if (cur->y == 1 && col == cur->x)
            return i;
        if (row == cur->y && col == cur->x)
            return ++i;
        if (str->text[i] == '\r')
        {
            row++;
            col = 1;
        }else{
            col++;
        }
        
        i++;
    }
}

//gets numb of rows
int get_max_rows(struct output *str)
{
    if (!str || !str->text) return 1;

    int rows = 1;
    for (int i = 0; i < str->lenght; i++)
        if (str->text[i] == '\r')
            rows++;

    return rows;
}

//find end of line in which cursor is located
int get_end_of_line(struct output * str,struct cursor * cur)
{
    int i = 0;
    int numb_of_r = 1;
    int col = 0;
    
    while(i<str->lenght)
    {
        if (str->text[i] == '\r')
        {
            if (cur->y == 1)
                return ++i;
            if (numb_of_r == cur->y)
            {                     
                return --col;
            }else{
                numb_of_r++;
                col = 0;
            }
        }
        col++;
        i++;
        if (i == str->lenght)
        {
            return --col;
        }
    }
}

// moving cursor to position and updating its position variable
void move_cursor(struct cursor * temp, int x, int y)
{
    temp->x += x;
    temp->y += y;
    int len = snprintf(NULL,0,"\x1b[%d;%dH", temp->y , temp->x);
    char * str = malloc(len + 1);
    sprintf(str,"\x1b[%d;%dH", temp->y, temp->x);

    write(STDOUT_FILENO,str,strlen(str));
    free(str);
}



//disabling  canon mode , echoing , and ctr-s /ctr-x
void disable_termosik()
{
    struct termios termosik_nowy;
    tcgetattr(STDIN_FILENO,&termosik_stary);
    termosik_nowy = termosik_stary;
    termosik_nowy.c_lflag &= ~(ICANON | ECHO | ISIG);
    termosik_nowy.c_iflag &= ~(IXON | ICRNL);
    termosik_nowy.c_oflag &= ~(OPOST);

    write(STDOUT_FILENO,"\x1b[?25h",strlen("\x1b[?25h"));
    tcsetattr(STDIN_FILENO,TCSANOW,&termosik_nowy);
}

// reseting terminal settings
void enable_termosik()
{
    tcsetattr(STDIN_FILENO,TCSANOW,&termosik_stary);
}

//save file
void saveFile(char ** file, struct output * out)
{

    write(STDOUT_FILENO,"\x1b[2J",4);
    write(STDOUT_FILENO,"\x1b[1;1H",6);

    enable_termosik();

    while (*file == NULL || (*file)[0] == '\n'){
        if (*file == NULL)
            *file = malloc(64);

        printf("Filename to save to:");
        fflush(stdout);
        fgets(*file, 64,stdin);
        if (*file[0] == '\n')
            continue;
        (*file)[strlen(*file)-1] = '\0';
    }
    

    FILE * f = fopen(*file,"w");
    fprintf(f,out->text);

    fclose(f);

    disable_termosik();
}


void main(int argc, char *argv[])
{
    
    char c;
    int temp_int = 0;
    struct output out = {NULL,0};
    struct cursor pos = {1,1};
    char * mouse_pos = NULL;
    char * file;
    char char_temp;
    char * string_temp;
    bool saved;

    if (argc > 1){
        if(argv[1][0] != '-')
        {
            file = argv[1];
            FILE * f = fopen(file,"r");
            string_temp = malloc(256);
            out.text = malloc(1);
            out.text[0] = '\0';
            while (fgets(string_temp,256,f) != NULL)
            {
                out.lenght += strlen(string_temp) + 1;
                out.text = realloc(out.text,out.lenght + 2);
                strcat(out.text,string_temp);
                if (out.text[out.lenght-2] == '\n')
                {
                    out.text[out.lenght-2] = '\r';
                    out.text[out.lenght-1] = '\n';
                    out.text[out.lenght] = '\0';
                }else{
                    out.lenght--;
                    out.text[out.lenght] = '\0';
                }

            }
            
            free(string_temp);
            fclose(f);
        }else{
            printf("Wrong argument given");
            exit(1);
        }
    }else{
        file = NULL;
    }

    
    disable_termosik();

    while (true)
    {   
        //clear terminal-> got to home pos -> display buffered text -> if valid input add char to buffer

        write(STDOUT_FILENO,"\x1b[2J",4);
        write(STDOUT_FILENO,"\x1b[1;1H",6);
        write(STDOUT_FILENO,out.text,out.lenght);
        move_cursor(&pos,0,0);
          
        char c = readKey();

        //ctrl-q -exit
        if (c == 17)
        {
            write(STDOUT_FILENO,"\x1b[2J",4);
            write(STDOUT_FILENO,"\x1b[1;1H",6);

            if (saved)
            {
                free(out.text);
                enable_termosik();
                exit(0);
            }else{
                write(STDOUT_FILENO,"Save before leaving? (Y/N):",28);
                char_temp = readKey();
                bool anwser = (char_temp == 'N' || char_temp == 'n' ) ? false : true;
                if (anwser){
                    
                    saveFile(&file,&out);
                    free(out.text);
                    enable_termosik();
                    exit(0);
                }
                
                    write(STDOUT_FILENO,"\x1b[2J",4);
                    write(STDOUT_FILENO,"\x1b[1;1H",6);

                enable_termosik();

                free(out.text);
                exit(0);
            }
            

        }
        // ctrl-s - save
        if ( c == 19) 
        {
            saveFile(&file,&out);
            saved = true;
            continue;
        }
        saved = false;
        // enter - new line
        if ( c == '\r' )
        {
            appendChar(&out,c,get_cursor_position(&out,&pos));
            pos.x = 1;
            pos.y++;
            continue;
        }
        //backspace - removing character at position of cursor
        if ( c == 8 || c == 127)
        {
            removeChar(&out,&pos,get_cursor_position(&out,&pos));
            if(out.text[get_cursor_position(&out,&pos)-1] == '\r')
            {
                removeChar(&out,&pos,get_cursor_position(&out,&pos));
            }
            continue;
        }
        // arrow handeling
        switch (c)
        {
        // up
        case 65:
            if (pos.y > 1)
            {
                pos.y--;
                temp_int = get_end_of_line(&out,&pos);
                pos.x = (pos.x < temp_int) ? pos.x : temp_int;
            }
            continue;  
        // down
        case 66: 
            pos.y++;
            temp_int = get_cursor_position(&out,&pos);
            if (get_max_rows(&out) < pos.y )
                {
                    pos.y--;
                    continue;
                }

            if ( temp_int < out.lenght)
            {
                pos.x = (pos.x < temp_int) ? pos.x : get_end_of_line(&out,&pos);
            }else{
                pos.x = get_end_of_line(&out,&pos);
            }
            continue; 
        //right
        case 67: 
            temp_int = get_cursor_position(&out,&pos);
            pos.x = (out.text[temp_int] != '\r' && temp_int < out.lenght) ? pos.x + 1 : pos.x; 
            continue; 
        // left
        case 68: 
            pos.x = (pos.x >1) ? pos.x - 1 : pos.x; 
            continue; 
        
        default: break;
        }
        
        //adding char and moving cursor x + 1
        appendChar(&out,c,get_cursor_position(&out,&pos));
        move_cursor(&pos,1,0);
        
    }
    write(STDOUT_FILENO,"\x1b[2J",4);
    write(STDOUT_FILENO,"\x1b[1;1H",6);

    enable_termosik();


    free(out.text);

}