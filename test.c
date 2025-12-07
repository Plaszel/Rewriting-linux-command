#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    write(STDOUT_FILENO, "\x1b[?25h", 6); // show cursor
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void editorDrawRows() {
    for (int i = 0; i < 24; i++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[?25l", 6);  // hide cursor
    write(STDOUT_FILENO, "\x1b[2J", 4);    // clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);     // cursor to home

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);     // move cursor again
    write(STDOUT_FILENO, "\x1b[?25h", 6);  // show cursor
}

int main() {
    enableRawMode();

    while (1) {
        editorRefreshScreen();
        
        char c;
        read(STDIN_FILENO, &c, 1);
        if (c == 'q') break;
    }
}
