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

// moving cursor to position and updating its position variable
void move_cursor(struct cursor * temp, int x , int y)
{
    int len = snprintf(NULL,0,"\x1b[%d;%dH", (temp->x + x), (temp->y + y));
    char * str = malloc(len + 1);
    sprintf(str,"\x1b[%d;%dH", (temp->x + x), (temp->y + y));

    write(STDOUT_FILENO,str,strlen(str));
    free(str);
    temp->x += x;
    temp->y += y;
};


//disabling  canon mode , echoing , and ctr-s /ctr-x
void disable_termosik()
{
    struct termios termosik_nowy;
    tcgetattr(STDIN_FILENO,&termosik_stary);
    termosik_nowy = termosik_stary;
    termosik_nowy.c_lflag &= ~(ICANON | ECHO);
    termosik_nowy.c_iflag &= ~(IXON);
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
        write(STDOUT_FILENO,"\x1b[?25h",strlen("\x1b[?25h"));
        write(STDOUT_FILENO,"\x1b[1;1H",strlen("\x1b[1;1H"));
        write(STDOUT_FILENO,out.text,out.lenght);
        read(STDIN_FILENO, &c, 1); 
        if ( c == 19) break;
        appendChar(&out,c);
        
    }

    enable_termosik();

}