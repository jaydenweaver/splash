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
    nodelay(stdscr, TRUE);
    raw();
    curs_set(0);
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
}

void close_renderer() { endwin(); }

void render_mt(const std::vector<int>& grid, int grid_width, int grid_height, int num_threads) {
    clear();

    int term_height, term_width;
    getmaxyx(stdscr, term_height, term_width);

    float cell_width  = static_cast<float>(grid_width) / term_width;
    float cell_height = static_cast<float>(grid_height) / term_height;

    // preallocate character buffer (one char per terminal cell)
    std::vector<char> frame(term_width * term_height);

    // parallel compute density -> char mapping
    auto worker = [&](int start_y, int end_y) {
        for (int y = start_y; y < end_y; ++y) {
            for (int x = 0; x < term_width; ++x) {
                int start_x = static_cast<int>(x * cell_width);
                int end_x   = std::min(grid_width, static_cast<int>((x + 1) * cell_width));
                int start_gy = static_cast<int>(y * cell_height);
                int end_gy   = std::min(grid_height, static_cast<int>((y + 1) * cell_height));

                int sum = 0;
                for (int gy = start_gy; gy < end_gy; ++gy)
                    for (int gx = start_x; gx < end_x; ++gx)
                        sum += grid[gy * grid_width + gx];

                frame[y * term_width + x] = density_to_char(sum);
            }
        }
    };

    std::vector<std::thread> threads;
    int chunk = (term_height + num_threads - 1) / num_threads;
    for (int t = 0; t < num_threads; t++) {
        int start_y = t * chunk;
        int end_y = std::min(term_height, start_y + chunk);
        if (start_y < end_y)
            threads.emplace_back(worker, start_y, end_y);
    }
    for (auto& th : threads) th.join();

    // draw to ncurses
    for (int y = 0; y < term_height; ++y) {
        for (int x = 0; x < term_width; ++x) {
            mvaddch(y, x, frame[y * term_width + x]);
        }
    }

    refresh();
}

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

            int sum = 0;
            for (int cy = start_y; cy < end_y; cy++) {
                for (int cx = start_x; cx < end_x; cx++) {
                    sum += grid[cy * grid_width + cx];
                }
            }
            char c = density_to_char(sum);

            mvaddch(y, x, c);
        }
    }

    refresh();
}