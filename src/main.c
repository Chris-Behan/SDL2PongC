#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600
#define true 1
#define false 0

int init_sdl(void);
int load_assets(void);
void initialize_players(void);
void game_loop(void);
void handle_events(SDL_Event *e);
void update_players();
void close();
int draw();

struct player
{
    struct SDL_Rect rect;
    int y_vel;
};

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
struct player g_player1;
struct player g_player2;
int g_velocity = 10;

main(int argc, char **argv)
{
    if (!init_sdl())
    {
        puts("Failed to initialize SDL");
    }
    else
    {
        if (!load_assets())
        {
            puts("Failed to load assets");
        }
        else
        {
            initialize_players();
            game_loop();
        }
        // Free resources and close SDL
        close();
        return 0;
    }
}

int init_sdl(void)
{
    int success = true;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize. SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        {
            puts("Warning: Linear texture filtering not enabled.");
        }
        g_window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (g_window == NULL)
        {
            printf("Window could not be created. SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (g_renderer == NULL)
            {
                printf("Could not create renderer. SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                int image_flags = IMG_INIT_PNG;
                if (!(IMG_Init(image_flags) & image_flags))
                {
                    printf("SDL_image could not initialize. SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
                SDL_RenderSetLogicalSize(g_renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
            }
        }
    }
    return success;
}

int load_assets()
{
    return true;
}

void initialize_players()
{
    int width = 20;
    int height = SCREEN_HEIGHT / 4;
    int x_pos1 = 0;
    int x_pos2 = SCREEN_WIDTH - width;
    int y_pos1 = SCREEN_HEIGHT / 2 - height / 2;
    int y_pos2 = SCREEN_HEIGHT / 2 - height / 2;

    g_player1 = (struct player){{x_pos1, y_pos1, width, height}, 0};
    g_player2 = (struct player){{x_pos2, y_pos2, width, height}, 0};
}

void game_loop()
{
    int quit = false;
    SDL_Event e;
    while (!quit)
    {
        //Handle events in queue
        while (SDL_PollEvent(&e) != 0)
        {
            // User requests to quit
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            handle_events(&e);
        }

        update_players();
        //Clear Screen
        SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(g_renderer);
        draw();
    }
}

void handle_events(SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN && e->key.repeat == 0)
    {
        switch (e->key.keysym.sym)
        {
        case SDLK_w:
            g_player1.y_vel -= g_velocity;
            break;
        case SDLK_s:
            g_player1.y_vel += g_velocity;
            break;
        case SDLK_UP:
            g_player2.y_vel -= g_velocity;
            break;
        case SDLK_DOWN:
            g_player2.y_vel += g_velocity;
            break;
        default:
            break;
        }
    }
    else if (e->type == SDL_KEYUP && e->key.repeat == 0)
    {
        switch (e->key.keysym.sym)
        {
        case SDLK_w:
            g_player1.y_vel += g_velocity;
            break;
        case SDLK_s:
            g_player1.y_vel -= g_velocity;
            break;
        case SDLK_UP:
            g_player2.y_vel += g_velocity;
            break;
        case SDLK_DOWN:
            g_player2.y_vel -= g_velocity;
            break;
        default:
            break;
        }
    }
}

void close()
{
    //Free loaded images
    //TODO
    // Destroy renderer
    SDL_DestroyRenderer(g_renderer);
    g_renderer = NULL;

    // Destroy Window
    SDL_DestroyWindow(g_window);
    g_window = NULL;

    // Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

void update_players()
{
    if (g_player1.y_vel >= 1)
    {
        g_player1.rect.y += g_player1.y_vel;
    }
    else if (g_player1.y_vel < 0)
    {
        g_player1.rect.y += g_player1.y_vel;
    }
    if (g_player2.y_vel >= 1)
    {
        g_player2.rect.y += g_player2.y_vel;
    }
    else if (g_player2.y_vel < 0)
    {
        g_player2.rect.y += g_player2.y_vel;
    }
}

int draw()
{
    // Draw player 1 read
    SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(g_renderer, &g_player1.rect);
    // Draw player 2 blue
    SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0xFF, 0xFF);
    SDL_RenderFillRect(g_renderer, &g_player2.rect);
    SDL_RenderPresent(g_renderer);
}
