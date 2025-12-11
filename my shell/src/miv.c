#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <stdbool.h>

/*
TODO 
-fix inserting chr inside string
*/



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
void appendChar( struct output * temp, char toadd)
{
    char * buff = realloc(temp->text,temp->lenght + 2);
    if (!buff) return;
    temp->text = buff;
    temp->text[temp->lenght] = toadd;
    temp->lenght++;
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
    if (!str->text || str->lenght == 0) return 1;

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

void main()
{
    char c;
    int temp_int = 0;
    struct output out = {NULL,0};
    struct cursor pos = {1,1};
    char * mouse_pos = NULL;
        
    disable_termosik();

    while (true)
    {   
        //clear terminal-> got to home pos -> display buffered text -> if valid input add char to buffer

        write(STDOUT_FILENO,"\x1b[2J",4);
        write(STDOUT_FILENO,"\x1b[1;1H",6);
        write(STDOUT_FILENO,out.text,out.lenght);
        move_cursor(&pos,0,0);
          
        char c = readKey();

        // ctrl-s - exit
        if ( c == 19) 
        {
            write(STDOUT_FILENO,"\x1b[2J",strlen("\x1b[2J")); 
            break;
        }
        // enter - new line
        if ( c == '\r' )
        {
            appendChar(&out,c);
            appendChar(&out,'\n');
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
        case 65:
            if (pos.y > 1)
            {
                pos.y--;
                temp_int = get_end_of_line(&out,&pos);
                pos.x = (pos.x < temp_int) ? pos.x : temp_int;
            }
            continue; 
            break;  
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
            break;
        case 67: 
            temp_int = get_cursor_position(&out,&pos);
            pos.x = (out.text[temp_int] != '\r' && temp_int < out.lenght) ? pos.x + 1 : pos.x; 
            continue; 
            break;
        case 68: 
            pos.x = (pos.x >1) ? pos.x - 1 : pos.x; 
            continue; 
            break;
        
        default: break;
        }
        
        //adding char and moving cursor x + 1
        appendChar(&out,c);
        move_cursor(&pos,1,0);
        
    }
    write(STDOUT_FILENO,"\x1b[2J",4);
    write(STDOUT_FILENO,"\x1b[1;1H",6);
    FILE * f = fopen("test.txt","w");
    fprintf(f,out.text);
    fclose(f);
    free(out.text);
    enable_termosik();

}