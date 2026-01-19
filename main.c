// Copyright 2025 Rivers Amyette
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 900
#define HEIGHT 600

#define LIMIT_FPS 1
#define TARGET_FPS 240
#define FRAME_TIME_MS (1000 / TARGET_FPS)

#define PARTICLE_COUNT 5000
#define PARTICLE_DAMPING 0.5f

#define MOUSE_MAX_SPEED 1000.0
#define MOUSE_IMPACT_RADIUS 50
#define MOUSE_DRAG_STRENGTH 10.0

typedef struct {
    float x, y;
    float vx, vy;
    Uint8 r, g, b;
} Particle;

int main(int argc, char* argv[])
{
    // Init and setup window & renderer
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(
        "SDL Particle Simulation",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );
    bool running = true;
    SDL_Event event;
    // First time for dt
    Uint32 last_time = SDL_GetTicks();

    // Seed rng
    srand(time(NULL));

    // Init particles
    Particle* particles = malloc(sizeof(Particle) * PARTICLE_COUNT);
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
        particles[i].vx = min_v + ((float)rand()) / (RAND_MAX + 1.0) * (max_v - min_v);
        particles[i].vy = min_v + ((float)rand()) / (RAND_MAX + 1.0) * (max_v - min_v);
        //particles[i].r = rand() % 256;
        //particles[i].g = rand() % 256;
        //particles[i].b = rand() % 256;
        particles[i].r = 0;
        particles[i].g = 255;
        particles[i].b = 100;
    }

    // Mouse input variables
    float mouse_x = 0;
    float mouse_y = 0;
    float mouse_vx = 0;
    float mouse_vy = 0;
    bool mouse_down = false;

    while (running)
    {
        // Track start of current frame to limit FPS
        Uint32 frame_start = SDL_GetTicks();

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
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        running = false;
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
                    float mouse_speed = SDL_sqrt(mouse_vx * mouse_vx + mouse_vy * mouse_vy);
                    if (mouse_speed > MOUSE_MAX_SPEED)
                    {
                        mouse_vx = (mouse_vx / mouse_speed) * MOUSE_MAX_SPEED;
                        mouse_vy = (mouse_vy / mouse_speed) * MOUSE_MAX_SPEED;
                    }
                    break;
            }
        }

        // Update
        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
            for (int j = 0; j < PARTICLE_COUNT; j++)
            {
                if (i == j) continue;
                float dx = particles[j].x - particles[i].x;
                float dy = particles[j].y - particles[i].y;
                float distance = SDL_sqrt((dx * dx) + (dy * dy));
                if (distance < 1.0f) distance = 1.0f;
                float force = -1000.0f / (distance * distance);
                particles[i].vx += (dx / distance) * force * dt;
                particles[i].vy += (dy / distance) * force * dt;
            }
            if (mouse_down)
            {
                float dx = mouse_x - particles[i].x;
                float dy = mouse_y - particles[i].y;
                float distance_to_mouse = SDL_sqrt((dx * dx) + (dy * dy));
                if (distance_to_mouse < MOUSE_IMPACT_RADIUS)
                {
                    float influence = 1 - (distance_to_mouse / MOUSE_IMPACT_RADIUS);
                    particles[i].vx += (mouse_vx - particles[i].vx) * influence * MOUSE_DRAG_STRENGTH * dt;
                    particles[i].vy += (mouse_vy - particles[i].vy) * influence * MOUSE_DRAG_STRENGTH * dt;
                }
            }
            particles[i].x += particles[i].vx * dt;
            particles[i].y += particles[i].vy * dt;
            if (particles[i].x < 0)
            {
                particles[i].x = 0;
                particles[i].vx *= -1;
            }
            if (particles[i].x > WIDTH - 1)
            {
                particles[i].x = WIDTH -1;
                particles[i].vx *= -1;
            }
            if (particles[i].y < 0)
            {
                particles[i].y = 0;
                particles[i].vy *= -1;
            }
            if (particles[i].y > HEIGHT - 1)
            {
                particles[i].y = HEIGHT -1;
                particles[i].vy *= -1;
            }
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

        // If we are limiting FPS, skip between frames
        #if LIMIT_FPS
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < FRAME_TIME_MS)
        {
            SDL_Delay(FRAME_TIME_MS - frame_time);
        }
        #endif
    }
    // Cleanup
    free(particles);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}