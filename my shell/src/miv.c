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
                cur->x = str->lenght;
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
        if (row == cur->y && col == cur->x)
            return i;
        if (str->text[i] == '\n')
        {
            row++;
            col = 1;
        }else{
            col++;
        }
        
        i++;
        return str->lenght;
    }
}

// moving cursor to position and updating its position variable
void move_cursor(struct cursor * temp, int x , int y)
{
    int len = snprintf(NULL,0,"\x1b[%d;%dH", (temp->y + y), (temp->x + x));
    char * str = malloc(len + 1);
    sprintf(str,"\x1b[%d;%dH", (temp->y + y), (temp->x + x));

    write(STDOUT_FILENO,str,strlen(str));
    free(str);
    temp->x += x;
    temp->y += y;
}



//disabling  canon mode , echoing , and ctr-s /ctr-x
void disable_termosik()
{
    struct termios termosik_nowy;
    tcgetattr(STDIN_FILENO,&termosik_stary);
    termosik_nowy = termosik_stary;
    termosik_nowy.c_lflag &= ~(ICANON | ECHO);
    termosik_nowy.c_iflag &= ~(IXON);
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
    struct output out = {NULL,0};
    struct cursor pos = {1,1};
    char * mouse_pos = NULL;
        
    disable_termosik();

    while (true)
    {   
        //clear terminal -> show cursor -> got to home pos -> display buffered text -> if valid input add char to buffer
        write(STDOUT_FILENO,"\x1b[2J",strlen("\x1b[2J"));
        write(STDOUT_FILENO,"\x1b[1;1H",strlen("\x1b[1;1H"));
        write(STDOUT_FILENO,out.text,out.lenght);
        move_cursor(&pos,0,0);
        
        read(STDIN_FILENO, &c, 1); 
        if ( c == 19) 
        {
            write(STDOUT_FILENO,"\x1b[2J",strlen("\x1b[2J")); 
            break;
        }
        if ( c == 10)
        {
            appendChar(&out,'\n');
            appendChar(&out,'\r');
            pos.x = 1;
            pos.y++;
            continue;
        }
        if ( c == 8 || c == 127)
        {
            removeChar(&out,&pos,get_cursor_position(&out,&pos));
            continue;
        }
        

        appendChar(&out,c);
        move_cursor(&pos,1,0);
        
    }
    FILE * f = fopen("test.txt","w");
    fprintf(f,out.text);
    fclose(f);
    free(out.text);
    enable_termosik();

}