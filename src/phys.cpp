#include <phys.h>

// simulation utilises an implementation of SPH - smooth particle hydrodynamics
// also involves spatial hashing to reduce runtime complexity | O(n^2) -> O(nm), n = particle count, m = neighbouring particle count

std::vector<Particle> particles;

void update(std::vector<int>& grid, int width, int height) {
    compute_density_pressure();
    compute_forces();
    integrate(width, height);
    fill(grid.begin(), grid.end(), 0);
    for (Particle p : particles) {
        grid[static_cast<int>(p.y) * width + static_cast<int>(p.x)]++;
    }
}

void spawn(int width, int height) {
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

void compute_density_pressure() {
    for (Particle& p : particles) {
        p.density = 0.0f;
        for(Particle& p2 : particles) {
            float dx = p2.x - p.x;
            float dy = p2.y - p.y;
            
            float r2 = dx * dx + dy * dy;
            p.density += MASS * density(r2);
        }
        p.pressure = STIFFNESS * std::max(p.density - DENSITY, 0.0f);
    }
}

void compute_forces() {
    for (Particle& p : particles) {
        float fx = 0.0f, fy = 0.0f;
        for (Particle& p2 : particles) {
            if (&p == &p2) continue;

            float dx = p2.x - p.x;
            float dy = p2.y - p.y;
            float r2 = dx * dx + dy * dy;
            float r = sqrt(r2);

            if (r < H && r > 0.0001f) {
                float p_term = (p.pressure / (p.density * p.density) + p2.pressure / (p2.density * p2.density));
                fx += MASS * p_term * pressure_gradient(r) * (dx / r);
                fy += MASS * p_term * pressure_gradient(r) * (dy / r);

                float vf = viscosity(r);
                fx += VISCOSITY * MASS * (p2.vx - p.vx) / p2.density * vf;
                fy += VISCOSITY * MASS * (p2.vy - p.vy) / p2.density * vf;
            }
        }
        fy += GRAVITY * p.density;
        if (p.density < 1e-6f) p.density = 1e-6f;

        p.ax = fx / p.density;
        p.ay = fy / p.density;

        if (std::isnan(p.ax) || std::isnan(p.ay))
            p.ax = p.ay = 0;
    }
}

void integrate(int width, int height) {
    for (Particle& p : particles) {
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
}
