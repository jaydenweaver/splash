#include <phys.h>
#include <renderer.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <iostream>

const int X_RES = 500;
const int Y_RES = 200;

int main(int argc, char* argv[]) {
    int run_time_ms = 0; // 0 is no limit

    if (argc > 1) {
        try {
            int seconds = std::stoi(argv[1]);
            run_time_ms = seconds * 1000;
        } catch (const std::invalid_argument&) {
            std::cerr << "invalid argument! must be a number.\n";
            return 1;
        } catch (const std::out_of_range&) {
            std::cerr << "invalid argument! number too large.\n";
            return 1;
        }
    }

    // 'high' resolution 2d grid for simulation - renderer downscales to fit
    // within CLI
    std::vector<int> grid(X_RES * Y_RES, 0);

    double tick_sum = 0; // tick time (ms)
    uint64_t ticks = 0;

    init_renderer();
    spawn(X_RES, Y_RES);
    MEVENT event;

    while (true) {

        if (run_time_ms > 0 && tick_sum > run_time_ms) break;

        int input = getch();
        if (input == 'q')
        break;

        if (input == KEY_MOUSE && getmouse(&event) == OK) {
        int terminal_width, terminal_height;
        getmaxyx(stdscr, terminal_height, terminal_width);
        if (event.bstate & BUTTON1_CLICKED) {
            explode((float)event.x * (static_cast<float>(X_RES) / terminal_width),
                    (float)event.y * static_cast<float>(Y_RES) / terminal_height);
        }
        }


        auto start = std::chrono::high_resolution_clock::now();
        update_mt(grid, X_RES, Y_RES);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> duration = end - start;
        tick_sum += duration.count();


        ticks++;

        render(grid, X_RES, Y_RES);
        //napms(8);
    }

    close_renderer();
    double avg_tick = tick_sum / static_cast<double>(ticks);
    double ticks_per_second = 1000.0 / avg_tick;
    int run_time = tick_sum / 1000;
    std::cout << "\n----------------- SPLASH -----------------\n" << std::endl;
    std::cout << "   average update tick time: " <<  avg_tick << " ms" << std::endl;
    std::cout << " average update ticks per second: " <<  ticks_per_second << "\n" << std::endl;
    std::cout << "            ran for " <<  run_time << " seconds." << std::endl;
    std::cout << "\n------------------------------------------\n" << std::endl;
    return 0;
}
