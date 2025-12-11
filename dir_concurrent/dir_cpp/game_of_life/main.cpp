#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <iostream>
#include <array>
#include <random>
#include <algorithm>
#include <cassert>
#include <thread>
#include <vector>
#include <mutex>

enum Colors {
    WHITE = 0xFFFFFFFF,
    BLACK = 0xFF000000,
    RED = 0xFFFF0000,
    GREEN = 0xFF00FF00,
    BLUE = 0xFF0000FF,
    YELLOW = 0xFFFFFF00,
    CYAN = 0xFF00FFFF,
    MAGENTA = 0xFFFF00FF,
    ORANGE = 0xFFFFA500,
    PURPLE = 0xFF800080
};

bool isAlive(const std::vector<std::vector<int>> &game, const int x, const int y, int G_GAME_W, int G_GAME_H) {
    int n_alive = 0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0)
                continue;
            int nx = x + i;
            int ny = y + j;
            if (nx >= 0 && nx < G_GAME_W && ny >= 0 && ny < G_GAME_H) {
                n_alive += game[nx][ny];
            }
        }
    }
    if (game[x][y] == 1 && (n_alive == 2 || n_alive == 3))
        return true;
    if (game[x][y] == 0 && n_alive == 3)
        return true;
    return false;
}

void updateDisplayAndPixels(int start, int end, const std::vector<std::vector<int>> &display, std::vector<std::vector<int>> &next_display, uint32_t *pixels, int G_GAME_W, int G_GAME_H) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);
    std::array<uint32_t, 10> colors = {static_cast<uint32_t>(WHITE), static_cast<uint32_t>(BLACK), static_cast<uint32_t>(RED), static_cast<uint32_t>(GREEN), static_cast<uint32_t>(BLUE), static_cast<uint32_t>(YELLOW), static_cast<uint32_t>(CYAN), static_cast<uint32_t>(MAGENTA), static_cast<uint32_t>(ORANGE), static_cast<uint32_t>(PURPLE)};
    for (int idx = start; idx < end; ++idx) {
        int i = idx / G_GAME_W;
        int j = idx % G_GAME_W;
        next_display[i][j] = isAlive(display, i, j, G_GAME_W, G_GAME_H) ? 1 : 0;
        pixels[j * G_GAME_W + i] = next_display[i][j] == 1 ? colors[dis(gen)] : WHITE;
    }
}

int main() {
    const int G_GAME_W = 1000;
    const int G_GAME_H = 1000;
    const int THREADS_N = std::thread::hardware_concurrency();
    int delay = 16; // Initial delay for ~60 FPS

    if (THREADS_N == 0) {
        std::cerr << "hardware_concurrency: 0";
        assert(THREADS_N != 0);
    } else if (THREADS_N <= 2) {
        std::cerr << "Can't run more than 2 threads";
        assert(THREADS_N > 2);
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, G_GAME_W, G_GAME_H, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Error creating window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Error creating renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, G_GAME_W, G_GAME_H);
    if (!texture) {
        std::cerr << "Error creating texture: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    auto *pixels = new uint32_t[G_GAME_W * G_GAME_H];

    bool quit = false;
    SDL_Event event;

    std::vector<std::vector<int>> display(G_GAME_W, std::vector<int>(G_GAME_H));
    std::vector<std::vector<int>> next_display(G_GAME_W, std::vector<int>(G_GAME_H));

    for (auto &row : display)
        std::generate(row.begin(), row.end(), []() { return rand() % 10 == 0 ? 1 : 0; });

    for (int i = 0; i < G_GAME_W * G_GAME_H; ++i) {
        pixels[i] = 0xFFFFFFFF;
    }

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_q) {
                    quit = true;
                } else if (event.key.keysym.sym == SDLK_w) {
                    delay = std::max(1, delay - 1); // Increase speed
                } else if (event.key.keysym.sym == SDLK_s) {
                    delay += 1; // Decrease speed
                }
            }
        }

        std::vector<std::thread> threads;
        int total_pixels = G_GAME_W * G_GAME_H;
        int size_per_thread = total_pixels / THREADS_N;
        int remaining_pixels = total_pixels % THREADS_N;

        for (int t = 0; t < THREADS_N; ++t) {
            int start = t * size_per_thread;
            int end = (t == THREADS_N - 1) ? (start + size_per_thread + remaining_pixels) : (start + size_per_thread);
            threads.emplace_back(updateDisplayAndPixels, start, end, std::ref(display), std::ref(next_display), pixels, G_GAME_W, G_GAME_H);
        }

        for (auto &t : threads) {
            t.join();
        }

        std::copy(next_display.begin(), next_display.end(), display.begin());

        SDL_UpdateTexture(texture, nullptr, pixels, G_GAME_W * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(delay);
    }

    delete[] pixels;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}