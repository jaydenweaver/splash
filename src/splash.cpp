#include <splash.h>

bool resized = false;
void handle_resize(int sig) {resized = true;}

int main() {
    signal(SIGWINCH, handle_resize);
    initscr();
    noecho();
    curs_set(0);

    int height, width;
    getmaxyx(stdscr, height, width);
    int draw = 0;

    while (true) {
        if (draw >= width) draw = 0;

        if (resized) {
            resized = false;
            endwin();
            refresh();
            getmaxyx(stdscr, height, width);
        }

        clear();  


        mvaddch(20, draw, '~');
        draw += 1;
        refresh();         // redraw screen
        usleep(40000);     // sleep 40ms
    }

    endwin();              
    return 0;
}