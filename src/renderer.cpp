#include <renderer.h>

char density_to_char(int density) {
    if (density < 1) return ' ';
    if (density < 2) return '.';
    if (density < 4) return ':';
    if (density < 20) return '=';
    if (density < 1500) return '#';
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
            for (int cy = start_y; cy < end_y; cy++) {
                for (int cx = start_x; cx < end_x; cx++) {
                    sum += grid[cy * grid_width + cx];
                    count++;
                }
            }

            int density_average = (count > 0) ? (sum / count) : 0;
            if (sum == 1) density_average = 1;
            char c = density_to_char(density_average);

            mvaddch(y, x, c);
        }
    }

    refresh();
}