//Using SDL and standard IO
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600
#define true 1
#define false 0
struct score
{
    struct SDL_Texture *texture;
    struct SDL_Rect rect;
    int val;
};

struct player
{
    struct SDL_Rect rect;
    int speed;
    int y_vel;
    SDL_Keycode up_key;
    SDL_Keycode down_key;
    struct score score;
};

struct ball
{
    struct SDL_Texture *texture;
    struct SDL_Rect rect;
    int x_vel;
    int y_vel;
};

int init_sdl(void);
int initialize_entities(void);
int load_font(SDL_Texture **, int *, int *);
int create_initial_score_texture(SDL_Texture **);
int create_score_texture(SDL_Texture **, SDL_Color, int);
struct ball create_ball(void);
void initialize_players(void);
int load_texture(char *, SDL_Texture **, int *, int *);
void game_loop(struct ball, struct player, struct player);
void handle_events(SDL_Event *e);
int get_y_velocity(SDL_Event *e, int, int, SDL_Keycode, SDL_Keycode);
int get_y_pos(struct player, int);
int updated_ball_x_velocity(struct ball, struct player, struct player);
int updated_ball_y_velocity(struct ball, struct player, struct player);
int calculate_y_velocity(struct ball, struct player);
int get_new_pos(int, int);
void handle_player_input(SDL_Event *e);
int x_collision(int, int, int);
int y_collision(int, int, int);
int load_text(char *, SDL_Texture **, int *, int *);

void handle_collisions(void);
void update_ball(void);
int can_move(struct player *);
void close_sdl();
void draw();

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
TTF_Font *g_font = NULL;
struct player g_player1;
struct player g_player2;
struct ball g_ball;
int g_velocity = 10;

int main(int argc, char **argv)
{
    if (!init_sdl())
    {
        puts("Failed to initialize SDL");
    }
    else
    {

        // Initialize ball
        SDL_Texture *ball_texture = NULL;
        int ball_width = 0;
        int ball_height = 0;
        if (!load_texture("src/assets/ball.png", &ball_texture, &ball_width, &ball_height))
        {
            puts("An error occurred while loading the ball texture");
        }
        struct ball ball = {ball_texture, {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, ball_width, ball_height}, 5, 0};

        // Initialize font for score
        g_font = TTF_OpenFont("src/assets/OpenSans-Bold.ttf", 28);
        if (g_font == NULL)
        {
            printf("Failed to load font, TTF Error: %s", TTF_GetError());
            return 1;
        }

        // Create player 1 score
        SDL_Texture *p1_score_texture = NULL;
        SDL_Color red = {255, 0, 0};
        if (!create_score_texture(&p1_score_texture, red, 0))
        {
            puts("Failed to create player 1 initial score texture");
            return 1;
        }
        struct score p1_score = {p1_score_texture, {200, 100, 50, 100}, 0};

        // Create player 2 score
        SDL_Texture *p2_score_texture = NULL;
        SDL_Color blue = {0, 255, 0};
        if (!create_score_texture(&p2_score_texture, red, 0))
        {
            puts("Failed to create player 2 initial score texture");
            return 1;
        }
        struct score p2_score = {p2_score_texture, {SCREEN_WIDTH - 100, 100, 100, 100}, 0};

        int width = 20;
        int height = SCREEN_HEIGHT / 4;
        int y_pos = SCREEN_HEIGHT / 2 - height / 2;
        int speed = 10;
        struct player player1 = {{80, y_pos, width, height}, 10, 0, SDLK_w, SDLK_s, p1_score};
        struct player player2 = {{SCREEN_WIDTH - 80, y_pos, width, height}, 10, 0, SDLK_UP, SDLK_DOWN, p2_score};
        game_loop(ball, player1, player2);

        // Free resources and close SDL
        close_sdl();
        return 0;
    }
}

int create_initial_score_texture(SDL_Texture **texture)
{
    int success = true;
    SDL_Color black = {0, 0, 0};
    SDL_Surface *surface = TTF_RenderText_Solid(g_font, "0", black);
    SDL_Texture *temp_texture = NULL;
    if (surface == NULL)
    {
        printf("failed to render initial text to surface, SDL_ttf Error: %s", TTF_GetError());
        success = false;
    }
    else
    {
        temp_texture = SDL_CreateTextureFromSurface(g_renderer, surface);
        if (temp_texture == NULL)
        {
            printf("Unable to create initial score texture from surface, SDL Error: %s", SDL_GetError());
            success = false;
        }
        else
        {
            *texture = temp_texture;
        }
        SDL_FreeSurface(surface);
    }
    return success;
}

int create_score_texture(SDL_Texture **texture, SDL_Color color, int score)
{
    int success = true;
    // Score will never exceed 5 chars (4 char + null terminator)
    char score_str[5];
    sprintf(score_str, "%d", score);
    SDL_Surface *surface = TTF_RenderText_Solid(g_font, score_str, color);
    SDL_Texture *temp_texture = NULL;
    if (surface == NULL)
    {
        printf("failed to render text to surface, SDL_ttf Error: %s", TTF_GetError());
        success = false;
    }
    else
    {
        temp_texture = SDL_CreateTextureFromSurface(g_renderer, surface);
        if (temp_texture == NULL)
        {
            printf("Unable to create initial score texture from surface, SDL Error: %s", SDL_GetError());
            success = false;
        }
        else
        {
            *texture = temp_texture;
        }
        SDL_FreeSurface(surface);
    }
    return success;
}

int load_font(SDL_Texture **texture, int *width, int *height)
{

    int success = true;
    TTF_Font *font = TTF_OpenFont("src/assets/lazy.ttf", 28);
    SDL_Color black = {0, 0, 0};
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, "hello", black);
    SDL_Texture *message = SDL_CreateTextureFromSurface(g_renderer, text_surface);
    SDL_Rect message_rect = {0, 0, 100, 100};
    SDL_FreeSurface(text_surface);
    SDL_RenderCopy(g_renderer, message, NULL, &message_rect);

    // g_font = TTF_OpenFont("src/assets/lazy.ttf", 28);
    // if (g_font == NULL)
    // {
    //     printf("Failed to load font, TTF Error: %s", TTF_GetError());
    //     success = false;
    // }
    // else
    // {
    //     if (!load_text("test", texture, width, height))
    //     {
    //         puts("Failed to load text");
    //         success = false;
    //     }
    // }
    // return success;
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
                if (TTF_Init() == -1)
                {
                    printf("SDL_ttf could not initialize. SDL_ttf Error: %s\n", TTF_GetError());
                    success = false;
                }
                SDL_RenderSetLogicalSize(g_renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
            }
        }
    }
    return success;
}

struct ball create_ball(void)
{
    char *path = "src/assets/ball.png";
    SDL_Texture *texture = NULL;
    int width = 0;
    int height = 0;
    if (!load_texture(path, &texture, &width, &height))
    {
        puts("An error occurred while loading the ball texture.");
    }
    else
    {
        // Initialize ball at to the middle of the screen on the left side with a velocity
        // moving to the right.
        return (struct ball){texture, {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, width, height}, 5, 0};
    }
}

int load_texture(char *path, SDL_Texture **texture, int *width, int *height)
{
    SDL_Surface *surface = IMG_Load(path);
    SDL_Texture *t = NULL;
    if (surface == NULL)
    {
        printf("Unable to load image %s. SDL_image Error: %s\n", path, IMG_GetError());
    }
    else
    {
        SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0xFF, 0xFF));
        t = SDL_CreateTextureFromSurface(g_renderer, surface);
        if (t == NULL)
        {
            printf("Unable to create texture from %s. SDL Error:%s\n", path, SDL_GetError());
        }
        else
        {
            *width = surface->w;
            *height = surface->h;
            *texture = t;
        }
        SDL_FreeSurface(surface);
    }
    int success = texture != NULL;
    return success;
}

void game_loop(struct ball ball, struct player p1, struct player p2)
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
            p1.y_vel = get_y_velocity(&e, p1.speed, p1.y_vel, p1.up_key, p1.down_key);
            p2.y_vel = get_y_velocity(&e, p2.speed, p2.y_vel, p2.up_key, p2.down_key);
        }

        p1.rect.y = get_y_pos(p1, SCREEN_HEIGHT);
        p2.rect.y = get_y_pos(p2, SCREEN_HEIGHT);
        ball.x_vel = updated_ball_x_velocity(ball, p1, p2);
        ball.y_vel = updated_ball_y_velocity(ball, p1, p2);
        ball.rect.x = get_new_pos(ball.rect.x, ball.x_vel);
        ball.rect.y = get_new_pos(ball.rect.y, ball.y_vel);
        handle_collisions();
        update_ball();
        //Clear Screen
        SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(g_renderer);
        // Draw ball and players to screen
        draw(ball, p1, p2);
    }
}

int get_y_velocity(SDL_Event *e, int speed, int current_y_vel, SDL_Keycode up_key, SDL_Keycode down_key)
{
    if (e->type == SDL_KEYDOWN && e->key.repeat == 0)
    {
        if (e->key.keysym.sym == up_key)
        {
            return current_y_vel - speed;
        }
        else if (e->key.keysym.sym == down_key)
        {
            return current_y_vel + speed;
        }
    }
    else if (e->type == SDL_KEYUP)
    {
        if (e->key.keysym.sym == up_key)
        {
            return current_y_vel + speed;
        }
        else if (e->key.keysym.sym == down_key)
        {
            return current_y_vel - speed;
        }
    }
    return current_y_vel;
}

void handle_collisions()
{
    if (g_player1.rect.x + g_player1.rect.w >= g_ball.rect.x && g_ball.rect.y >= g_player1.rect.y && g_ball.rect.y <= g_player1.rect.y + g_player1.rect.h && g_ball.x_vel < 0)
    {
        g_ball.x_vel *= -1;
    }
    else if (g_ball.rect.x >= g_player2.rect.x - g_player2.rect.w && g_ball.rect.y >= g_player2.rect.y && g_ball.rect.y <= g_player2.rect.y + g_player2.rect.h && g_ball.x_vel > 0)
    {
        g_ball.x_vel *= -1;
    }
}

int x_collision(int a_pos, int a_vel, int b_pos)
{
    if (a_vel > 0)
    {
        if (a_pos >= b_pos)
        {
            return true;
        }
        return false;
    }
    else if (a_vel < 0)
    {
        if (a_pos <= b_pos)
        {
            return true;
        }
        return false;
    }
    return false;
}

int y_collision(int a_pos, int b_top, int b_bottom)
{
    if (a_pos >= b_top && a_pos <= b_bottom)
    {
        return true;
    }
    return false;
}

void close_sdl()
{
    //Free loaded images
    //TODO
    // Destroy renderer
    SDL_DestroyRenderer(g_renderer);
    g_renderer = NULL;

    // Destroy Window
    SDL_DestroyWindow(g_window);
    g_window = NULL;

    // Destroy font
    TTF_CloseFont(g_font);
    g_font = NULL;

    // Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

int get_y_pos(struct player p, int screen_height)
{
    int new_pos = p.rect.y + p.y_vel;
    if (new_pos >= 0 && new_pos <= screen_height - p.rect.h)
    {
        return new_pos;
    }
    return p.rect.y;
}

int updated_ball_x_velocity(struct ball b, struct player p1, struct player p2)
{
    if (p1.rect.x + p1.rect.w >= b.rect.x && b.rect.y + b.rect.h >= p1.rect.y && b.rect.y - b.rect.h <= p1.rect.y + p1.rect.h && b.x_vel < 0)
    {
        return b.x_vel *= -1;
    }
    else if (b.rect.x >= p2.rect.x - p2.rect.w && b.rect.y + b.rect.h >= p2.rect.y && b.rect.y - b.rect.h <= p2.rect.y + p2.rect.h && b.x_vel > 0)
    {
        return b.x_vel *= -1;
    }
    return b.x_vel;
}

int updated_ball_y_velocity(struct ball b, struct player p1, struct player p2)
{
    if (p1.rect.x + p1.rect.w >= b.rect.x && b.rect.y + b.rect.h >= p1.rect.y && b.rect.y - b.rect.h <= p1.rect.y + p1.rect.h)
    {
        return calculate_y_velocity(b, p1);
    }
    else if (b.rect.x >= p2.rect.x - p2.rect.w && b.rect.y + b.rect.h >= p2.rect.y && b.rect.y - b.rect.h <= p2.rect.y + p2.rect.h)
    {
        return calculate_y_velocity(b, p2);
    }
    else if (b.rect.y <= 0)
    {
        return b.y_vel *= -1;
    }
    else if (b.rect.y + b.rect.h >= SCREEN_HEIGHT)
    {
        return b.y_vel *= -1;
    }
    return b.y_vel;
}

int calculate_y_velocity(struct ball b, struct player p)
{
    int mid_paddle = p.rect.y + p.rect.h / 2;

    float speed = 5.0f;
    float multiplier = 0.0f;
    if (b.rect.y > mid_paddle)
    {
        multiplier = (b.rect.y - mid_paddle) / (float)(p.rect.h / 2);
    }
    else if (b.rect.y < mid_paddle)
    {
        multiplier = (b.rect.y - mid_paddle) / (float)(p.rect.h / 2);
    }
    return speed * multiplier;
}

int get_new_pos(int current, int velocity)
{
    return current + velocity;
}

void update_ball()
{
    g_ball.rect.x += g_ball.x_vel;
    g_ball.rect.y += g_ball.y_vel;
}

int can_move(struct player *player)
{
    // If moving down, since y coordinate grows downward on screen
    if (player->y_vel > 0)
    {
        if (player->rect.y + player->rect.h >= SCREEN_HEIGHT)
        {
            return false;
        }
    }
    // moving up, since y coordinate is 0 at top of screen
    else if (player->y_vel < 0)
    {
        if (player->rect.y <= 0)
        {
            return false;
        }
    }
    return true;
}

int load_text(char *text, SDL_Texture **texture, int *width, int *height)
{

    SDL_Color color = {0, 0, 0};
    TTF_Font *font = NULL;
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *t = NULL;
    if (text_surface == NULL)
    {
        printf("Unable to render text surface, SDL_ttf Error: %s\n", TTF_GetError());
    }
    else
    {
        t = SDL_CreateTextureFromSurface(g_renderer, text_surface);
        if (t == NULL)
        {
            printf("Unable to load rendered text surface to texture, SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            *width = text_surface->w;
            *height = text_surface->h;
            *texture = t;
        }
        SDL_FreeSurface(text_surface);
    }
    int success = texture != NULL;
    return success;
}

void draw(struct ball ball, struct player p1, struct player p2)
{
    // Draw player 1 red
    SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(g_renderer, &p1.rect);

    // Draw Player 1 score
    SDL_RenderCopy(g_renderer, p1.score.texture, NULL, &p1.score.rect);

    // Draw player 2 blue
    SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0xFF, 0xFF);
    SDL_RenderFillRect(g_renderer, &p2.rect);
    // Draw ball
    SDL_RenderCopy(g_renderer, ball.texture, NULL, &ball.rect);

    // SDL_Texture *text_texture;
    // int text_width;
    // int text_height;
    // SDL_Rect text_rect = {20, 20, 20, 20};
    // load_font(text_texture, text_width, text_height);

    // TTF_Font *font = TTF_OpenFont("src/assets/lazy.ttf", 28);
    // SDL_Color black = {0, 0, 0};
    // SDL_Surface *text_surface = TTF_RenderText_Solid(font, "hello", black);
    // SDL_Texture *message = SDL_CreateTextureFromSurface(g_renderer, message);
    // SDL_Rect message_rect = {0, 0, 100, 100};
    // SDL_RenderCopy(g_renderer, message, NULL, &message_rect);

    SDL_RenderPresent(g_renderer);
}
