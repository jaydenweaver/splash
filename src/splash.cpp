#include <phys.h>
#include <renderer.h>
#include <unistd.h>
#include <vector>

const int X_RES = 1000;
const int Y_RES = 500;

int main() {
    // 'high' resolution 2d grid for simulation - renderer downscales to fit within CLI
    std::vector<int> grid(X_RES * Y_RES, 0);

    init_renderer();
    while (true) {
    randomise_grid(grid, X_RES, Y_RES);
        render(grid, X_RES, Y_RES);
        usleep(40000);
    }
    
    close_renderer();
    return 0;
}