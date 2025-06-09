#include <phys.h>
#include <renderer.h>
#include <unistd.h>
#include <vector>

const int X_RES = 500;
const int Y_RES = 200;

int main() {
    // 'high' resolution 2d grid for simulation - renderer downscales to fit within CLI
    std::vector<int> grid(X_RES * Y_RES, 0);

    init_renderer();
    spawn(X_RES, Y_RES);
    MEVENT event;

    while (true) {
        int input = getch();
        if (input == 'q') break;

        if (input == KEY_MOUSE && getmouse(&event) == OK) {
            int terminal_width, terminal_height;
            getmaxyx(stdscr, terminal_height, terminal_width);
            if (event.bstate & BUTTON1_CLICKED) {
                explode((float) event.x * (static_cast<float>(X_RES) / terminal_width), (float) event.y * static_cast<float>(Y_RES) / terminal_height);
            }
        }

        update(grid, X_RES, Y_RES);
        render(grid, X_RES, Y_RES);
        napms(8);
    }
    
    close_renderer();
    return 0;
}