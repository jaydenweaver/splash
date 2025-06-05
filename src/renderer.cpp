#include <renderer.h>

char density_to_char(int density) {
    if (density < 1) return ' ';
    if (density < 2) return '.';
    if (density < 4) return ':';
    if (density < 8) return '=';
    if (density < 15) return '#';
    return '#';
}

void init_renderer() {
    initscr();
    noecho();
    curs_set(0);
}

void close_renderer() { endwin(); }

void render(const std::vector<int>& grid, int grid_width, int grid_height) {
    clear();
    int height, width;
    getmaxyx(stdscr, height, width);

    float cell_width = static_cast<float>(grid_width) / width;
    float cell_height = static_cast<float>(grid_height) / height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int start_x = static_cast<int>(x * cell_width);
            int end_x = std::min(grid_width, static_cast<int>((x + 1) * cell_width));
            int start_y = static_cast<int>(y * cell_height);
            int end_y = std::min(grid_height, static_cast<int>((y + 1) * cell_height));

            int sum = 0, count = 0;
            for (int cell_y = start_y; cell_y < end_y; cell_y++) {
                for (int cell_x = start_x; cell_x < end_x; cell_x++) {
                    sum += grid[cell_y * grid_width + cell_x];
                    count++;
                }
            }

            int density_average = sum;
            char c = density_to_char(density_average);

            mvaddch(y, x, c);
        }
    }

    refresh();
}