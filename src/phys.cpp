#include <phys.h>

const int RANDOM_COUNT = 200;

void randomise_grid(std::vector<int>& grid, int height, int width) {
    grid = std::vector<int>(height * width, 0);
    for (int i = 0; i < RANDOM_COUNT; i++) {
        int rand_x = rand() % width;
        int rand_y = rand() % height;

        grid[rand_y * width + rand_x] = rand() % 20;
    }
}