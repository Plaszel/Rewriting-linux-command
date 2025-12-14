#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <stdbool.h>

struct termios termosik_stary;

struct output
{
    char *text;
    int lenght;
};

struct cursor
{
    int x;
    int y;
};

enum Special_keys_enum
{
    KEY_ARROW_UP = 1001,
    KEY_ARROW_DOWN = 1002,
    KEY_ARROW_RIGHT = 1003,
    KEY_ARROW_LEFT = 1004,
    KEY_HOME = 1005,
    KEY_END = 1006,
    KEY_PG_UP = 1007,
    KEY_PG_DOWN = 1008,
};

int Special_keys[128] = {
    ['A'] = KEY_ARROW_UP,
    ['B'] = KEY_ARROW_DOWN,
    ['C'] = KEY_ARROW_RIGHT,
    ['D'] = KEY_ARROW_LEFT,
    ['H'] = KEY_HOME,
    ['F'] = KEY_END,
    ['5'] = KEY_PG_UP,
    ['6'] = KEY_PG_DOWN,
};

int readKey();

void appendChar(struct output *temp, char toadd, int pos);

void removeChar(struct output *str, struct cursor *cur, int pos);

int get_cursor_position(struct output *str, struct cursor *cur);

int get_max_rows(struct output *str);

int get_end_of_line(struct output *str, struct cursor *cur);

void move_cursor(struct cursor *temp, int x, int y);

void EnableRawDataMode();

void DisableRawDataMode();

// read key input
int readKey()
{
    char input[4];
    read(STDIN_FILENO, &input, 4);
    if (input[0] == '\x1B')
    {
        if (input[1] == '[')
        {
            return Special_keys[input[2]];
        }
    }
    else
    {
        return input[0];
    }
}

// adding single char to output
void appendChar(struct output *temp, char toadd, int pos)
{
    char *buff = realloc(temp->text, temp->lenght + 2);
    if (!buff)
        return;
    temp->text = buff;
    memmove(&temp->text[pos + 1], &temp->text[pos], temp->lenght - pos);
    temp->text[pos] = toadd;
    temp->lenght++;
    temp->text[temp->lenght] = '\0';
}

// removing char at position
void removeChar(struct output *str, struct cursor *cur, int pos)
{
    if (cur->x == 1 && cur->y == 1)
    {
        return;
    }

    if (cur->x == 1)
    {
        cur->y--;
        if (cur->y == 1)
        {
            cur->x = pos;
        }
        else
        {
            int i = 1;
            while (str->lenght > 0 && str->text[pos - 1 - i] != '\n')
            {
                i++;
            }
            cur->x = i;
        }
    }
    else
    {
        cur->x--;
    }

    memmove(&str->text[pos], &str->text[pos + 1], str->lenght - pos);
    str->lenght--;
    str->text = realloc(str->text, str->lenght + 1);
    str->text[str->lenght] = '\0';
    if (str->text[str->lenght - 1] == '\r')
    {
        removeChar(str, cur, str->lenght - 1);
    }
}

// get cursor position (lenght)
int get_cursor_position(struct output *str, struct cursor *cur)
{
    if (!str->text || str->lenght == 0)
        return 0;

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
        }
        else
        {
            col++;
        }

        i++;
    }
}

// gets numb of rows
int get_max_rows(struct output *str)
{
    if (!str || !str->text)
        return 1;

    int rows = 1;
    for (int i = 0; i < str->lenght; i++)
        if (str->text[i] == '\r')
            rows++;

    return rows;
}

// find end of line in which cursor is located
int get_end_of_line(struct output *str, struct cursor *cur)
{
    int i = 0;
    int numb_of_r = 1;
    int col = 0;

    while (i < str->lenght)
    {
        if (str->text[i] == '\r')
        {
            if (cur->y == 1)
                return ++i;
            if (numb_of_r == cur->y)
            {
                return --col;
            }
            else
            {
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
void move_cursor(struct cursor *temp, int x, int y)
{
    temp->x += x;
    temp->y += y;
    int len = snprintf(NULL, 0, "\x1b[%d;%dH", temp->y, temp->x);
    char *str = malloc(len + 1);
    sprintf(str, "\x1b[%d;%dH", temp->y, temp->x);

    write(STDOUT_FILENO, str, strlen(str));
    free(str);
}

// disabling  canon mode , echoing , and ctr-s /ctr-x
void EnableRawDataMode()
{
    struct termios termosik_nowy;
    tcgetattr(STDIN_FILENO, &termosik_stary);
    termosik_nowy = termosik_stary;
    termosik_nowy.c_lflag &= ~(ICANON | ECHO | ISIG);
    termosik_nowy.c_iflag &= ~(IXON | ICRNL);
    termosik_nowy.c_oflag &= ~(OPOST);

    write(STDOUT_FILENO, "\x1b[?25h", strlen("\x1b[?25h"));
    tcsetattr(STDIN_FILENO, TCSANOW, &termosik_nowy);
}

// reseting terminal settings
void DisableRawDataMode()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &termosik_stary);
}

// clear screen
static void clearScreen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[1;1H", 6);
}

static void endProgram(struct output *out)
{
    free(out->text);
    clearScreen();
    DisableRawDataMode();
    exit(0);
}

// save file
void saveFile(char **file, struct output *out)
{
    clearScreen();
    DisableRawDataMode();

    while (*file == NULL || (*file)[0] == '\n')
    {
        if (*file == NULL)
            *file = malloc(64);

        printf("Filename to save to:");
        fflush(stdout);
        fgets(*file, 64, stdin);
        if ((*file)[0] == '\n')
            continue;
        (*file)[strlen(*file) - 1] = '\0';
    }

    FILE *f = fopen(*file, "w");
    fprintf(f, out->text);

    fclose(f);

    EnableRawDataMode();
}

// read file
void readFile(char **file, char *filename, struct output *out)
{
    char *string_temp = malloc(256);
    *file = filename;

    FILE *f = fopen(*file, "r");
    out->text = malloc(1);
    out->text[0] = '\0';
    while (fgets(string_temp, 256, f) != NULL)
    {
        out->lenght += strlen(string_temp);
        out->text = realloc(out->text, out->lenght + 1);
        
        if (string_temp[strlen(string_temp)] == '\n' )
        {
            string_temp[out->lenght + 1] = '\r';
            out->lenght++;
        }
        else
        {
            string_temp[out->lenght + 1] = '\0';
        }
        strcat(out->text, string_temp);
    }

    free(string_temp);
    fclose(f);
}

// quit terminal
void quitTerminal(bool saved, struct output *out, char *file)
{
    clearScreen();
    char char_temp;

    if (saved)
    {
        endProgram(out);
    }
    else
    {
        write(STDOUT_FILENO, "Save before leaving? (Y/N):", 28);
        char_temp = readKey();
        bool anwser = (char_temp == 'N' || char_temp == 'n') ? false : true;

        if (anwser)
        {
            saveFile(&file, out);
        }

        endProgram(out);
    }
}

// Arrow keys
void moveUP(struct cursor *pos, struct output *out)
{
    if (pos->y > 1)
    {
        int temp_int = 0;
        pos->y--;
        temp_int = get_end_of_line(out, pos);
        pos->x = (pos->x < temp_int) ? pos->x : temp_int;
    }
}

void moveDown(struct cursor *pos, struct output *out)
{
    int temp_int = 0;
    pos->y++;
    temp_int = get_cursor_position(out, pos);

    if (get_max_rows(out) < pos->y)
        pos->y--;

    if (temp_int < out->lenght)
    {
        pos->x = (pos->x < temp_int) ? pos->x : get_end_of_line(out, pos);
    }
    else
    {
        pos->x = get_end_of_line(out, pos);
    }
}

void moveRight(struct cursor *pos, struct output *out)
{
    int temp_int = 0;
    temp_int = get_cursor_position(out, pos);
    pos->x = (out->text[temp_int] != '\r' && temp_int < out->lenght) ? pos->x + 1 : pos->x;
}

void moveLeft(struct cursor *pos)
{
    pos->x = (pos->x > 1) ? pos->x - 1 : pos->x;
}

//pg up/down keys
void pgUP(struct cursor *pos, struct output *out)
{
    int temp_int = 0;
    for (int i = 0; i < 50; i++)
    {
        if (!(pos->y > 1))
            break;
        pos->y--;
    }
    temp_int = get_end_of_line(out, pos);
    pos->x = (pos->x < temp_int) ? pos->x : temp_int;
}

void pgDown(struct cursor *pos, struct output *out)
{
    int temp_int = get_max_rows(out);
    for (int i = 0; i < 50; i++)
    {
        if (!(pos->y < temp_int))
            break;
        pos->y++;
    }
    temp_int = get_cursor_position(out, pos);
    if (temp_int < out->lenght)
    {
        pos->x = (pos->x < temp_int) ? pos->x : get_end_of_line(out, pos);
    }
    else
    {
        pos->x = get_end_of_line(out, pos);
    }
}

void main(int argc, char *argv[])
{
    int temp_int = 0;
    char c;
    struct output out = {NULL, 0};
    struct cursor pos = {1, 1};
    char *mouse_pos = NULL;
    char *file;
    bool saved;

    // get filename if given
    if (argc > 1)
    {
        if (argv[1][0] != '-')
        {
            readFile(&file, argv[1], &out);
        }
        else
        {
            printf("Wrong argument given");
            exit(1);
        }
    }
    else
    {
        file = NULL;
    }

    EnableRawDataMode();

    while (true)
    {

        clearScreen();

        // displaying text and reseting cursor
        write(STDOUT_FILENO, out.text, out.lenght);
        move_cursor(&pos, 0, 0);

        int c = readKey();

        // ctrl-q -exit
        if (c == 17)
        {
            quitTerminal(saved, &out, file);
        }

        // ctrl-s - save
        if (c == 19)
        {
            saveFile(&file, &out);
            saved = true;
            continue;
        }
        saved = false;

        // enter - new line
        if (c == '\r')
        {
            appendChar(&out, '\n', get_cursor_position(&out, &pos));
            appendChar(&out, '\r', get_cursor_position(&out, &pos));
            pos.x = 1;
            pos.y++;
            continue;
        }

        // backspace - removing character at position of cursor
        if (c == 8 || c == 127)
        {
            removeChar(&out, &pos, get_cursor_position(&out, &pos));
            continue;
        }

        // up arrow
        if (c == KEY_ARROW_UP)
        {
            moveUP(&pos, &out);
            continue;
        }

        // down arrow
        if (c == KEY_ARROW_DOWN)
        {
            moveDown(&pos, &out);
            continue;
        }

        // right arrow
        if (c == KEY_ARROW_RIGHT)
        {
            moveRight(&pos, &out);
            continue;
        }

        // left arrow
        if (c == KEY_ARROW_LEFT)
        {
            moveLeft(&pos);
            continue;
        }

        // home key
        if (c == KEY_HOME)
        {
            pos.x = 1;
            continue;
        }

        // end key
        if (c == KEY_END)
        {
            pos.x = get_end_of_line(&out,&pos);
            continue;
        }

        // key pg up
        if (c == KEY_PG_UP)
        {
            pgUP(&pos, &out);
            continue;
        }

        // key pg down
        if (c == KEY_PG_DOWN)
        {
            pgDown(&pos, &out);
            continue;
        }

        // adding char and moving cursor x + 1
        appendChar(&out, c, get_cursor_position(&out, &pos));
        move_cursor(&pos, 1, 0);
    }
}