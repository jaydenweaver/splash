#include <phys.h>
#include <renderer.h>
#include <unistd.h>

bool resized = false;
void handle_resize(int sig) {resized = true;}

int main() {

    while (true) {       // redraw screen
        usleep(40000);     // sleep 40ms
    }
            
    return 0;
}