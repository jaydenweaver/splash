#include <phys.h>
#include <renderer.h>
#include <unistd.h>
#include <vector>

const int X_RES = 500;
const int Y_RES = 200;

int main() {
    // 'high' resolution 2d grid for simulation - renderer downscales to fit within CLI
    std::vector<int> grid(X_RES * Y_RES, 0);

    init_renderer();
    spawn(X_RES, Y_RES);
    while (true) {
        update(grid, X_RES, Y_RES);
        render(grid, X_RES, Y_RES);
        napms(16);
    }
    
    close_renderer();
    return 0;
}