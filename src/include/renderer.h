#pragma once

#include <ncurses.h>
#include <thread>
#include <vector>
#include <algorithm>


void init_renderer();
void render_mt(const std::vector<int>&, int, int, int);
void render(const std::vector<int>&, int, int);
void close_renderer();
