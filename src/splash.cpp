#include <phys.h>
#include <renderer.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <iostream>

const int X_RES = 500;
const int Y_RES = 200;

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

void run_simulation(int num_threads, uint64_t total_ticks) {
    std::vector<int> grid(X_RES * Y_RES, 0);
    double tick_sum = 0.0;
    uint64_t ticks = 0;

    MEVENT event;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (ticks < total_ticks) {
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

        auto tick_start = std::chrono::high_resolution_clock::now();

        if (num_threads > 1)
            update_mt(grid, X_RES, Y_RES, num_threads);
        else
            update_sequential(grid, X_RES, Y_RES);

        auto tick_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tick_time = tick_end - tick_start;
        tick_sum += tick_time.count();
        ticks++;

        //render(grid, X_RES, Y_RES);
        //napms(8); // optional frame delay
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double runtime_sec = std::chrono::duration<double>(end_time - start_time).count();
    double avg_tick = tick_sum / static_cast<double>(ticks);
    double ticks_per_second = 1000.0 / avg_tick;

    std::cout << "\n------------------------------------------\n";
    std::cout << (num_threads > 1 ? " PARALLEL RUN COMPLETE\n" : " SEQUENTIAL RUN COMPLETE\n");
    std::cout << " Total ticks: " << ticks << "\n";
    std::cout << " Average tick time: " << avg_tick << " ms\n";
    std::cout << " Average ticks per second: " << ticks_per_second << "\n";
    std::cout << " Total runtime: " << runtime_sec << " seconds\n";
    std::cout << "------------------------------------------\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <num_ticks>\n";
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

    //init_renderer();

    bool particles_match = true;

    std::cout << "\n=== running sequential version (" << num_ticks << " ticks, " << num_particles << " particles) ===\n";
    spawn(X_RES, Y_RES); // reset particles
    run_simulation(1, num_ticks);
    std::vector<Particle> particles_seq = get_particles();

    std::cout << "\n=== running parallel version (" << num_ticks << " ticks, " << num_particles << " particles, 2 threads) ===\n";
    spawn(X_RES, Y_RES); // reset particles
    run_simulation(2, num_ticks);
    std::vector<Particle> particles_par = get_particles();
    particles_match = compare_particles(particles_seq, particles_par);

    std::cout << "\n=== running parallel version (" << num_ticks << " ticks, " << num_particles << " particles, 4 threads) ===\n";
    spawn(X_RES, Y_RES); // reset particles
    run_simulation(4, num_ticks);
    particles_par = get_particles();
    if (particles_match) particles_match = compare_particles(particles_seq, particles_par);

    std::cout << "\n=== running parallel version (" << num_ticks << " ticks, " << num_particles << " particles, 8 threads) ===\n";
    spawn(X_RES, Y_RES); // reset particles
    run_simulation(8, num_ticks);
    particles_par = get_particles();
    if (particles_match) particles_match = compare_particles(particles_seq, particles_par);
    
    std::cout << "\n=== running parallel version (" << num_ticks << " ticks, " << num_particles << " particles, 16 threads) ===\n";
    spawn(X_RES, Y_RES); // reset particles
    run_simulation(16, num_ticks);
    particles_par = get_particles();
    if (particles_match) particles_match = compare_particles(particles_seq, particles_par);

    //close_renderer();

    std::cout << "\nSimulation finished.\n";

    if (particles_match) {
        std::cout << "Particle states match.\n";
    } else {
        std::cout << "Particle states differ.\n";
    }
        return 0;
}
