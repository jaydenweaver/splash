#pragma once
#include <vector>
#include <cmath>
#include <ncurses.h>
#include <iostream>

// simulation constants
//-----------------------------------
constexpr int PARTICLE_COUNT = 2000;
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
//-----------------------------------

struct Particle {
    float x, y;
    float vx = 0, vy = 0;
    float ax = 0, ay = 0;
    float density = 0;
    float pressure = 0;
};

void update(std::vector<int>&, int, int);
void spawn(int, int);

float density(float);
float pressure_gradient(float);
float viscosity(float);

void compute_density_pressure();
void compute_forces();
void integrate(int, int);

void explode(float, float);
