#include <phys.h>

// simulation utilises an implementation of SPH - smooth particle hydrodynamics

// parallel_for: passes (index, thread_id)
template <typename Func>
void parallel_for(size_t count, Func&& func, unsigned int num_threads) {
    if (num_threads == 0) num_threads = 1;
    std::vector<std::thread> threads;
    size_t chunk_size = (count + num_threads - 1) / num_threads;

    for (unsigned int t = 0; t < num_threads; ++t) {
        size_t start = t * chunk_size;
        size_t end = std::min(count, start + chunk_size);
        if (start >= end) break;
        threads.emplace_back([=, &func]() {
            for (size_t i = start; i < end; ++i) func(i, t);
        });
    }
    for (auto &th : threads) th.join();
}

struct cell_key_hash {
    size_t operator()(const cell_key& k) const {
        return std::hash<int>()(k.first) ^ (std::hash<int>()(k.second << 1));
    }
};

std::unordered_map<cell_key, std::vector<Particle*>, cell_key_hash> spatial_map;
std::vector<Particle> particles;

std::vector<Particle> get_particles() {
    return particles;
}

void update_sequential(std::vector<int>& grid, int width, int height) {
    spatial_map.clear();

    for (Particle& p : particles) {
        int cx = get_cell_x(p, SPATIAL_CELL);
        int cy = get_cell_y(p, SPATIAL_CELL);
        spatial_map[{cx, cy}].push_back(&p);
    }

    compute_density_pressure();
    compute_forces();
    integrate(width, height);

    fill(grid.begin(), grid.end(), 0);
    for (const Particle& p : particles) {
        grid[static_cast<int>(p.y) * width + static_cast<int>(p.x)]++;
    }
}

void update_mt(std::vector<int>& grid, int width, int height, int num_threads) {
    // clear global spatial map
    spatial_map.clear();

    //unsigned int num_threads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::unordered_map<cell_key, std::vector<Particle*>, cell_key_hash>> local_maps;
    local_maps.resize(num_threads);

    // reserve 
    size_t approx_cells = (particles.size() / (SPATIAL_CELL * SPATIAL_CELL)) + 1;
    for (auto &lm : local_maps) lm.reserve(std::min<size_t>(approx_cells, 1024));

    // build spatial maps in parallel, each thread writes to its own local_maps[t]
    parallel_for(particles.size(),
        [&](size_t i, unsigned int t) {
            Particle& p = particles[i];
            int cx = get_cell_x(p, SPATIAL_CELL);
            int cy = get_cell_y(p, SPATIAL_CELL);
            local_maps[t][{cx, cy}].push_back(&p);
        },
        num_threads
    );

    // merge local maps into global spatial_map
    for (auto &lm : local_maps) {
        for (auto &entry : lm) {
            auto &key = entry.first;
            auto &vec = entry.second;
            auto &dest = spatial_map[key];               // creates bucket if missing
            dest.insert(dest.end(), vec.begin(), vec.end());
        }
    }

    // parallel per-particle computation (reads spatial_map, writes only the particle)
    parallel_for(particles.size(),
        [&](size_t i, unsigned int) {
            compute_density_pressure_for_particle(particles[i]);
        },
        num_threads
    );

    parallel_for(particles.size(),
        [&](size_t i, unsigned int) {
            compute_forces_for_particle(particles[i]);
        },
        num_threads
    );

    parallel_for(particles.size(),
        [&](size_t i, unsigned int) {
            integrate_particle(particles[i], width, height);
        },
        num_threads
    );

    // fill the output grid, use per-thread local histograms then merge to avoid atomics
    // create per-thread local int arrays sized to grid; merge at end.
    size_t grid_size = static_cast<size_t>(width) * static_cast<size_t>(height);
    std::vector<std::vector<int>> thread_histograms(num_threads, std::vector<int>(grid_size, 0));

    parallel_for(particles.size(),
        [&](size_t i, unsigned int t) {
            const Particle &p = particles[i];
            int xi = static_cast<int>(p.x);
            int yi = static_cast<int>(p.y);
            if (xi >= 0 && xi < width && yi >= 0 && yi < height) {
                size_t idx = static_cast<size_t>(yi) * width + xi;
                thread_histograms[t][idx] += 1;
            }
        },
        num_threads
    );

    // reduce thread histograms into grid (single-threaded)
    std::fill(grid.begin(), grid.end(), 0);
    for (unsigned int t = 0; t < num_threads; ++t) {
        auto &h = thread_histograms[t];
        for (size_t idx = 0; idx < grid_size; ++idx) {
            if (h[idx] != 0) grid[idx] += h[idx];
        }
    }
}

void spawn(int width, int height) {
    particles.clear();
    srand(1234);
    int count = 0;
    for (int y = 0; y < width && count < PARTICLE_COUNT; y++) {
        for (int x = 0; x < height && count < PARTICLE_COUNT; x++) {
            particles.push_back({static_cast<float>(x), static_cast<float>(y), 0.1f * (rand() % 10 - 5), 0.0f});
            count++;
        }
    }
}

// kernel functions
//----------------------------------------------------------------------------------------------

float density(float r2) {
    float h2 = H * H;
    if (r2 >= 0 && r2 <= h2) 
        return 315.0f / (64.0f * M_PI * pow(H, 9)) * pow(h2 - r2, 3);
    return 0.0f;
}

float pressure_gradient(float r) {
    if (r >= 0 && r <= H)
        return -45.0f / (M_PI * pow(H, 6)) * pow(H - r, 2);
    return 0.0f;
}

float viscosity(float r) {
    if (r >= 0 && r <= H) 
        return 45.0f / (M_PI * pow(H, 6)) * (H - r);
    return 0.0f;
}

// computations
//----------------------------------------------------------------------------------------------

void compute_density_pressure_for_particle(Particle& p) {
    p.density = 0.0f;
    int cx = get_cell_x(p, SPATIAL_CELL);
    int cy = get_cell_y(p, SPATIAL_CELL);

    for (int jx = -1; jx <= 1; jx++) {
        for (int jy = -1; jy <= 1; jy++) {
            cell_key key = {cx + jx, cy + jy};
            if (spatial_map.find(key) == spatial_map.end()) continue;

            for (Particle* p2 : spatial_map[key]) {
                float dx = p2->x - p.x;
                float dy = p2->y - p.y;
                float r2 = dx * dx + dy * dy;
                p.density += MASS * density(r2);
            }
        }
    }
    p.pressure = STIFFNESS * std::max(p.density - DENSITY, 0.0f);
}

void compute_forces_for_particle(Particle& p) {
    float fx = 0.0f, fy = 0.0f;
    int cx = get_cell_x(p, SPATIAL_CELL);
    int cy = get_cell_y(p, SPATIAL_CELL);

    for (int jx = -1; jx <= 1; jx++) {
        for (int jy = -1; jy <= 1; jy++) {
            cell_key key = {cx + jx, cy + jy};
            if (spatial_map.find(key) == spatial_map.end()) continue;

            for (Particle* p2 : spatial_map[key]) {
                if (&p == p2) continue;

                float dx = p2->x - p.x;
                float dy = p2->y - p.y;
                float r2 = dx * dx + dy * dy;
                float r = sqrt(r2);

                if (r < H && r > 0.0001f) {
                    float p_term = (p.pressure / (p.density * p.density) + p2->pressure / (p2->density * p2->density));
                    fx += MASS * p_term * pressure_gradient(r) * (dx / r);
                    fy += MASS * p_term * pressure_gradient(r) * (dy / r);

                    float vf = viscosity(r);
                    fx += VISCOSITY * MASS * (p2->vx - p.vx) / p2->density * vf;
                    fy += VISCOSITY * MASS * (p2->vy - p.vy) / p2->density * vf;
                }
            }
        }
    }

    fy += GRAVITY * p.density;
    if (p.density < 1e-6f) p.density = 1e-6f;

    p.ax = fx / p.density;
    p.ay = fy / p.density;

    if (std::isnan(p.ax) || std::isnan(p.ay))
        p.ax = p.ay = 0;
}

void integrate_particle(Particle& p, int width, int height) {
    p.vx += DT * p.ax;
    p.vy += DT * p.ay;

    p.x += DT * p.vx;
    p.y += DT * p.vy;
        
    // bounds check
    if (p.x < 0) {
        p.x = 0;
        p.vx *= -1.0f * BOUNCE_DAMPENING;
    }
    if (p.x > width - 1) {
        p.x = width - 1;
        p.vx *= -1.0f * BOUNCE_DAMPENING;
    }
    if (p.y < 0) {
        p.y = 0;
        p.vy *= -1.0f * BOUNCE_DAMPENING;
    }
    if (p.y > height - 1) {
        p.y = height - 1;
        p.vy *= -1.0f * BOUNCE_DAMPENING;
    }
}

void compute_density_pressure() {
    for (Particle& p : particles) {
        compute_density_pressure_for_particle(p);
    }
}

void compute_forces() {
    for (Particle& p : particles) {
        compute_forces_for_particle(p);
    }
}

void integrate(int width, int height) {
    for (Particle& p : particles) {
        integrate_particle(p, width, height);
    }
}


void explode(float x, float y) {
    for (Particle& p : particles) {
        float dx = p.x - x;
        float dy = p.y - y;
        float r = sqrt(dx * dx + dy * dy);

        if (r < BLAST_RADIUS && r > 0.0001f) {
            float nx = dx / r;
            float ny = dy / r;
            float force = BLAST_STRENGTH * (1.0f - r / BLAST_RADIUS);
            p.vx += nx * force;
            p.vy += ny * force;
        }
    }
}

int get_cell_x(Particle& p, int cell_size) {
    return floor(p.x / cell_size);
}

int get_cell_y(Particle& p, int cell_size) {
    return floor(p.y / cell_size);
}
