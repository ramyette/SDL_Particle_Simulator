// Copyright 2025 Rivers Amyette

/*
 * This file is what I used to export this project
 * to html using Emscripten to be playable on my website
 */

#include <SDL2/SDL.h>
#include <emscripten.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 900
#define HEIGHT 600

#define LIMIT_FPS 1
#define TARGET_FPS 240
#define FRAME_TIME_MS (1000 / TARGET_FPS)

#define PARTICLE_COUNT 50000
#define PARTICLE_DAMPING 0.5f

#define MOUSE_MAX_SPEED 1000.0
#define MOUSE_IMPACT_RADIUS 50
#define MOUSE_DRAG_STRENGTH 10.0

typedef struct {
    float x, y;
    float vx, vy;
    Uint8 r, g, b;
} Particle;

// Variables moved to outside for use in the new gameloop
SDL_Window* window;
SDL_Renderer* renderer;
Particle* particles;
SDL_Event event;
bool running = true;
Uint32 last_time = 0;
float mouse_x = 0;
float mouse_y = 0;
float mouse_vx = 0;
float mouse_vy = 0;
bool mouse_down = false;

// Web does not allow while(running) for gameloop, so I'm using this function instead
void frame(void)
{
    // Delta time
    Uint32 current_time = SDL_GetTicks();
    float dt = (current_time - last_time) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;
    last_time = current_time;

    // Reset mouse velocity
    mouse_vx = 0.0f;
    mouse_vy = 0.0f;

    // Input
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                emscripten_cancel_main_loop();
                return;

            case SDL_KEYUP:
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    emscripten_cancel_main_loop();
                    return;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_down = true;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_down = false;
                }
                break;

            case SDL_MOUSEMOTION:
                mouse_x = (float)event.motion.x;
                mouse_y = (float)event.motion.y;

                if (dt > 0.0001f)
                {
                    mouse_vx = (float)event.motion.xrel / dt;
                    mouse_vy = (float)event.motion.yrel / dt;
                }
                else
                {
                    mouse_vx = 0.0f;
                    mouse_vy = 0.0f;
                }

                // Cap mouse velocity
                {
                    float mouse_speed = SDL_sqrt(mouse_vx * mouse_vx + mouse_vy * mouse_vy);
                    if (mouse_speed > MOUSE_MAX_SPEED)
                    {
                        mouse_vx = (mouse_vx / mouse_speed) * MOUSE_MAX_SPEED;
                        mouse_vy = (mouse_vy / mouse_speed) * MOUSE_MAX_SPEED;
                    }
                }
                break;
        }
    }

    // Update
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        if (mouse_down)
        {
            float dx = mouse_x - particles[i].x;
            float dy = mouse_y - particles[i].y;
            float distance_to_mouse = SDL_sqrt((dx * dx) + (dy * dy));
            if (distance_to_mouse < MOUSE_IMPACT_RADIUS)
            {
                float influence = 1.0f - (distance_to_mouse / MOUSE_IMPACT_RADIUS);
                particles[i].vx += (mouse_vx - particles[i].vx) * influence * MOUSE_DRAG_STRENGTH * dt;
                particles[i].vy += (mouse_vy - particles[i].vy) * influence * MOUSE_DRAG_STRENGTH * dt;
            }
        }

        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;

        if (particles[i].x < 0) { particles[i].x = 0; particles[i].vx *= -1; }
        if (particles[i].x > WIDTH - 1) { particles[i].x = WIDTH - 1; particles[i].vx *= -1; }
        if (particles[i].y < 0) { particles[i].y = 0; particles[i].vy *= -1; }
        if (particles[i].y > HEIGHT - 1) { particles[i].y = HEIGHT - 1; particles[i].vy *= -1; }

        particles[i].vx *= 1.0f - PARTICLE_DAMPING * dt;
        particles[i].vy *= 1.0f - PARTICLE_DAMPING * dt;
    }

    // Render
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        SDL_SetRenderDrawColor(renderer, particles[i].r, particles[i].g, particles[i].b, 255);
        SDL_RenderDrawPoint(renderer, particles[i].x, particles[i].y);
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[])
{
    // Init and setup window & renderer
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(
        "SDL Particle Simulation",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_SOFTWARE
    );

    // Use logical resolution so resize/input/render stay aligned
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    running = true;

    // First time for dt
    last_time = SDL_GetTicks();

    // Seed rng
    srand(time(NULL));

    // Init particles
    particles = malloc(sizeof(Particle) * PARTICLE_COUNT);
    if (!particles)
    {
        printf("Failed to allocate particles with size %d\n", PARTICLE_COUNT);
        return 1;
    }

    // Max and min particle starting velocities
    float max_v = 50.0f;
    float min_v = -50.0f;
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        particles[i].x = (rand() % (WIDTH - 1)) + 1;
        particles[i].y = (rand() % (HEIGHT - 1)) + 1;
        particles[i].vx = min_v + ((float)rand() / (float)RAND_MAX) * (max_v - min_v);
        particles[i].vy = min_v + ((float)rand() / (float)RAND_MAX) * (max_v - min_v);
        particles[i].r = rand() % 256;
        particles[i].g = rand() % 256;
        particles[i].b = rand() % 256;
    }

    // Mouse input variables
    mouse_x = 0;
    mouse_y = 0;
    mouse_vx = 0;
    mouse_vy = 0;
    mouse_down = false;

    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}