#pragma once

#include <ncurses.h>
#include <vector>

void init_renderer();
void render(const std::vector<int>&, int, int);
void close_renderer();
