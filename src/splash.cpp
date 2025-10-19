#include <phys.h>
#include <renderer.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <cstring>

const int X_RES = 500;
const int Y_RES = 200;

struct SimulationResult {
    int threads;
    uint64_t ticks;
    double avg_tick_ms;
    double ticks_per_second;
    double total_runtime_s;
    bool matches_baseline;
};

bool compare_particles(const std::vector<Particle>& a, const std::vector<Particle>& b, float eps = 1e-5f) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        const Particle& p1 = a[i];
        const Particle& p2 = b[i];
        if (fabs(p1.x - p2.x) > eps || fabs(p1.y - p2.y) > eps ||
            fabs(p1.vx - p2.vx) > eps || fabs(p1.vy - p2.vy) > eps ||
            fabs(p1.density - p2.density) > eps || fabs(p1.pressure - p2.pressure) > eps)
            return false;
    }
    return true;
}

SimulationResult run_simulation(int num_threads, uint64_t total_ticks) {
    std::vector<int> grid(X_RES * Y_RES, 0);
    double tick_sum = 0.0;
    uint64_t ticks = 0;

    MEVENT event;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (ticks < total_ticks) {
        int input = getch();
        if (input == 'q') break;

        if (input == KEY_MOUSE && getmouse(&event) == OK) {
            int terminal_width, terminal_height;
            getmaxyx(stdscr, terminal_height, terminal_width);
            if (event.bstate & BUTTON1_CLICKED) {
                explode((float)event.x * (static_cast<float>(X_RES) / terminal_width),
                        (float)event.y * static_cast<float>(Y_RES) / terminal_height);
            }
        }

        auto tick_start = std::chrono::high_resolution_clock::now();

        if (num_threads > 1) {
            update_mt(grid, X_RES, Y_RES, num_threads);
            render_mt(grid, X_RES, Y_RES, num_threads);
        } else {
            update_sequential(grid, X_RES, Y_RES);
            render(grid, X_RES, Y_RES);
        }

        auto tick_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tick_time = tick_end - tick_start;
        tick_sum += tick_time.count();
        ticks++;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double runtime_sec = std::chrono::duration<double>(end_time - start_time).count();
    double avg_tick = tick_sum / static_cast<double>(ticks);
    double ticks_per_second = 1000.0 / avg_tick;

    return {num_threads, ticks, avg_tick, ticks_per_second, runtime_sec, true};
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <num_ticks> [num_particles]\n";
        return 1;
    }

    uint64_t num_ticks = 0;
    uint64_t num_particles = 5000; // default
    try {
        num_ticks = std::stoull(argv[1]);
        if (num_ticks == 0) throw std::invalid_argument("zero ticks");
    } catch (...) {
        std::cerr << "Invalid argument: must be a positive integer tick count.\n";
        return 1;
    }

    if (argc > 2) {
        try {
            num_particles = std::stoull(argv[2]);
            if (num_particles <= 0)
                throw std::invalid_argument("Number of particles must be positive");
        } catch (...) {
            std::cerr << "Invalid number of particles. Must be a positive integer.\n";
            return 1;
        }
    }

    set_particle_count(num_particles);
    init_renderer();

    std::vector<SimulationResult> results;
    std::vector<int> thread_counts = {1, 2, 4, 8, 16};
    std::vector<Particle> baseline;

    for (int threads : thread_counts) {
        spawn(X_RES, Y_RES);
        auto result = run_simulation(threads, num_ticks);
        std::vector<Particle> current = get_particles();

        if (threads == 1) {
            baseline = current;
        } else {
            result.matches_baseline = compare_particles(baseline, current);
        }

        results.push_back(result);
    }

    close_renderer();

    // Print results table
    std::cout << "\nSimulation complete (" << num_ticks << " ticks, " << num_particles << " particles)\n\n";
    std::cout << std::left
              << std::setw(10) << "Threads"
              << std::setw(15) << "Avg Tick (ms)"
              << std::setw(18) << "Ticks/sec"
              << std::setw(18) << "Total Runtime (s)"
              << std::setw(12) << "Matches"
              << "\n";

    std::cout << std::string(70, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::left
                  << std::setw(10) << r.threads
                  << std::setw(15) << std::fixed << std::setprecision(3) << r.avg_tick_ms
                  << std::setw(18) << std::fixed << std::setprecision(2) << r.ticks_per_second
                  << std::setw(18) << std::fixed << std::setprecision(2) << r.total_runtime_s
                  << std::setw(12) << (r.matches_baseline ? "Yes" : "No")
                  << "\n";
    }

    std::cout << std::string(70, '-') << "\n";
    std::cout << "Done.\n";

    return 0;
}
