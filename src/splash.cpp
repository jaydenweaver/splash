#include <phys.h>
#include <renderer.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <iostream>

const int X_RES = 500;
const int Y_RES = 200;

int main() {
    // performance benchmarking
    using clock = std::chrono::high_resolution_clock;
    uint64_t total_frames = 0;
    auto start_time = clock::now();

    // 'high' resolution 2d grid for simulation - renderer downscales to fit within CLI
    std::vector<int> grid(X_RES * Y_RES, 0);

    init_renderer();
    spawn(X_RES, Y_RES);
    MEVENT event;

    while (true) {
        int input = getch();
        if (input == 'q') break;

        total_frames++;

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

    auto end_time = clock::now();
    
    std::chrono::duration<double, std::milli> time_taken_ms = end_time - start_time;

    // average frame time in ms
    double avg_frame_time_ms = time_taken_ms.count() / total_frames;

    // total time in seconds
    double time_taken_s = time_taken_ms.count() / 1000.0;

    // average FPS
    double avg_fps = total_frames / time_taken_s;

    std::cout << "average frame time: " << avg_frame_time_ms << " ms" << std::endl;
    std::cout << "average fps: " << avg_fps << std::endl;

    return 0;
}