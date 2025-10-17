#pragma once
#include <vector>
#include <cmath>
#include <ncurses.h>
#include <iostream>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <atomic>
#include <algorithm>
#include <mutex>

// simulation constants
//-----------------------------------
constexpr int PARTICLE_COUNT = 10000;
constexpr float DENSITY = 0.02f;
constexpr float STIFFNESS = 100.0f;
constexpr float H = 8.0f;
constexpr float MASS = 1.0f;
constexpr float VISCOSITY = 0.1f;
constexpr float DT = 0.03f;
constexpr float GRAVITY = 50.0f;
constexpr float BOUNCE_DAMPENING = 0.2f;
constexpr float BLAST_RADIUS = 100.0f;
constexpr float BLAST_STRENGTH = 250.0f;
// length and width of cells in grid used for spatial hashing
constexpr int SPATIAL_CELL = 20;
//-----------------------------------

struct Particle {
    float x, y;
    float vx = 0, vy = 0;
    float ax = 0, ay = 0;
    float density = 0;
    float pressure = 0;
};

using cell_key = std::pair<int, int>;

void update_sequential(std::vector<int>& grid, int width, int height)
void update_mt(std::vector<int>&, int, int);
void spawn(int, int);

float density(float);
float pressure_gradient(float);
float viscosity(float);

void compute_density_pressure_for_particle(Particle& p);
void compute_forces_for_particle(Particle& p);
void integrate_particle(Particle& p, int width, int height);

void explode(float, float);

int get_cell_x(Particle& , int);
int get_cell_y(Particle& , int);