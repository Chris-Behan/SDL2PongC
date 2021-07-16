// Using SDL and standard IO
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60
#define true 1
#define false 0

typedef struct
{
    struct SDL_Texture *texture;
    struct SDL_Rect rect;
    struct SDL_Color color;
    int val;
} Score;

typedef struct
{
    struct SDL_Rect rect;
    int speed;
    int y_vel;
    SDL_Keycode up_key;
    SDL_Keycode down_key;
    Score score;
    Mix_Chunk *sound;
} Player;

typedef struct
{
    struct SDL_Texture *texture;
    struct SDL_Rect rect;
    float x_vel;
    float y_vel;
} Ball;

int init_sdl(void);
int initialize_entities(void);
int load_font(SDL_Texture **, int *, int *);
int create_initial_score_texture(SDL_Texture **);
Score create_score(int, int, SDL_Color, int);
int create_score_texture(SDL_Texture **, SDL_Color, int, int *, int *);
Ball create_ball(void);
void initialize_players(void);
int load_texture(char *, SDL_Texture **, int *, int *);
void game_loop(Ball, Player, Player);
void handle_events(SDL_Event *e);
int get_y_velocity(SDL_Event *e, int, int, SDL_Keycode, SDL_Keycode);
int get_y_pos(int y_pos, int vel, int player_height);
int updated_ball_x_velocity(Ball, Player, Player);
int updated_ball_y_velocity(Ball, Player, Player);
float calculate_y_velocity(SDL_Rect ball, SDL_Rect player);
int get_new_pos(int, float);
void handle_player_input(SDL_Event *e);
int load_text(char *, SDL_Texture **, int *, int *);

void handle_collisions(void);
void update_ball(Ball *b, Player p1, Player p2);
void handle_score(Player *, Player *, Ball *);
void handle_score2(Player *, Player *, Ball *);
int can_move(Player *);
void close_sdl();
void draw();

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
TTF_Font *g_font = NULL;

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
        if (!load_texture(
                "src/assets/ball.png", &ball_texture, &ball_width, &ball_height))
        {
            puts("An error occurred while loading the ball texture");
        }
        Ball ball = {
            ball_texture,
            {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 10, ball_width, ball_height},
            7,
            0};

        // Initialize font for score
        g_font = TTF_OpenFont("src/assets/OpenSans-Bold.ttf", 48);
        if (g_font == NULL)
        {
            printf("Failed to load font, TTF Error: %s", TTF_GetError());
            return 1;
        }

        // Create player 1 score
        SDL_Color red = {255, 0, 0};
        int p1_score_x = 200;
        int p1_score_y = 100;
        Score p1_score = create_score(p1_score_x, p1_score_y, red, 0);

        // Create player 2 score
        SDL_Color blue = {0, 0, 255};
        int p2_score_x = SCREEN_WIDTH - 200;
        int p2_score_y = 100;
        Score p2_score = create_score(p2_score_x, p2_score_y, blue, 0);

        // Load audio files
        Mix_Chunk *p1_sound = Mix_LoadMUS("src/assets/paddle_1_sound.ogg");
        Mix_Chunk *p2_sound = Mix_LoadMUS("src/assets/paddle_2_sound.ogg");

        int width = 20;
        int height = SCREEN_HEIGHT / 6;
        int y_pos = SCREEN_HEIGHT / 2 - height / 2;
        int speed = 10;
        Player player1 = {{0, y_pos, width, height}, 20, 0, SDLK_w, SDLK_s, p1_score, p1_sound};
        Player player2 = {{SCREEN_WIDTH - width, y_pos, width, height}, 20, 0, SDLK_UP, SDLK_DOWN, p2_score, p2_sound};
        game_loop(ball, player1, player2);

        // Free resources and close SDL
        close_sdl(&ball, &player1, &player2);
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
        printf("failed to render initial text to surface, SDL_ttf Error: %s",
               TTF_GetError());
        success = false;
    }
    else
    {
        temp_texture = SDL_CreateTextureFromSurface(g_renderer, surface);
        if (temp_texture == NULL)
        {
            printf(
                "Unable to create initial score texture from surface, SDL Error: %s",
                SDL_GetError());
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

Score create_score(int x_pos, int y_pos, SDL_Color color, int val)
{
    SDL_Texture *score_texture = NULL;
    int width = 0;
    int height = 0;
    if (!create_score_texture(&score_texture, color, 0, &width, &height))
    {
        puts("Failed to create score texture");
        return;
    }
    Score score = {score_texture, {x_pos, y_pos, width, height}, color, val};
    return score;
}

int create_score_texture(SDL_Texture **texture,
                         SDL_Color color,
                         int score,
                         int *width,
                         int *height)
{
    int success = true;
    // Score will never exceed 5 chars (4 char + null terminator)
    char score_str[5];
    sprintf(score_str, "%d", score);
    SDL_Surface *surface = TTF_RenderText_Solid(g_font, score_str, color);
    SDL_Texture *temp_texture = NULL;
    if (surface == NULL)
    {
        printf("failed to render text to surface, SDL_ttf Error: %s",
               TTF_GetError());
        success = false;
    }
    else
    {
        temp_texture = SDL_CreateTextureFromSurface(g_renderer, surface);
        if (temp_texture == NULL)
        {
            printf(
                "Unable to create initial score texture from surface, SDL Error: %s",
                SDL_GetError());
            success = false;
        }
        else
        {
            *texture = temp_texture;
            *width = surface->w;
            *height = surface->h;
        }
        SDL_FreeSurface(surface);
    }
    return success;
}

int load_font(SDL_Texture **texture, int *width, int *height)
{
    int success = true;
    TTF_Font *font = TTF_OpenFont("src/assets/OpenSans-Bold.ttf", 28);
    SDL_Color black = {0, 0, 0};
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, "hello", black);
    SDL_Texture *message = SDL_CreateTextureFromSurface(g_renderer, text_surface);
    SDL_Rect message_rect = {0, 0, 100, 100};
    SDL_FreeSurface(text_surface);
    SDL_RenderCopy(g_renderer, message, NULL, &message_rect);
}

int init_sdl(void)
{
    int success = true;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
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

        // Initialize SDL Mixer
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        {
            printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
            success = false;
        }

        g_window = SDL_CreateWindow("Pong",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    SCREEN_WIDTH,
                                    SCREEN_HEIGHT,
                                    SDL_WINDOW_SHOWN);
        if (g_window == NULL)
        {
            printf("Window could not be created. SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            g_renderer = SDL_CreateRenderer(
                g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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
                    printf("SDL_image could not initialize. SDL_image Error: %s\n",
                           IMG_GetError());
                    success = false;
                }
                if (TTF_Init() == -1)
                {
                    printf("SDL_ttf could not initialize. SDL_ttf Error: %s\n",
                           TTF_GetError());
                    success = false;
                }
                SDL_RenderSetLogicalSize(g_renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
            }
        }
    }
    return success;
}

Ball create_ball(void)
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
        // Initialize ball at to the middle of the screen on the left side with a
        // velocity moving to the right.
        return (Ball){texture, {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, width, height}, 5, 0};
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

void game_loop(Ball ball, Player p1, Player p2)
{
    int quit = false;
    SDL_Event e;
    int millisecond_per_frame = 1000 / FPS;
    // Get current time elapsed in-order to enforce FPS limit
    int next_game_step = SDL_GetTicks();
    while (!quit)
    {
        int now = SDL_GetTicks();
        // Only enter game logic if within frame limit
        if (next_game_step < now)
        {
            // Handle events in queue
            while (SDL_PollEvent(&e) != 0)
            {
                // User requests to quit
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
                process_input(&e, &p1, &p2);
            }

            update_state(&ball, &p1, &p2);
            draw(ball, p1, p2);

            // Increment game step counter by desired frames per second
            next_game_step += millisecond_per_frame;
        }
        else
        {
            // Sleep if cycle finishes early
            SDL_Delay(next_game_step - now);
        }
    }
}

// Update the y_vel of players depending on the input.
void process_input(SDL_Event *e, Player *p1, Player *p2)
{
    update_y_vel(e, p1);
    update_y_vel(e, p2);
}

void update_y_vel(SDL_Event *e, Player *p)
{
    // Set y_vel to player speed when key pressed
    if (e->type == SDL_KEYDOWN && e->key.repeat == 0)
    {
        if (e->key.keysym.sym == p->up_key)
        {
            p->y_vel = -abs(p->speed);
        }
        else if (e->key.keysym.sym == p->down_key)
        {
            p->y_vel = abs(p->speed);
        }
    }
    // Set y_vel to 0 when key released
    else if (e->type == SDL_KEYUP)
    {
        if (e->key.keysym.sym == p->up_key)
        {
            p->y_vel = 0;
        }
        else if (e->key.keysym.sym == p->down_key)
        {
            p->y_vel = 0;
        }
    }
}

void update_state(Ball *b, Player *p1, Player *p2)
{
    update_players(p1, p2);
    update_ball(b, *p1, *p2);
    handle_score(p1, p2, b);
}

// Updates the position of the players.
void update_players(Player *p1, Player *p2)
{
    // update player 1 position
    p1->rect.y = get_y_pos(p1->rect.y, p1->y_vel, p1->rect.h);
    // update player 2 position
    p2->rect.y = get_y_pos(p2->rect.y, p2->y_vel, p2->rect.h);
}

// Checks if someone scores and updates the game state accordingly.
// Player who scores will have their score incremented, and the
// position of the ball will be reset to the middle of the screen
// with its velocity in the direction toward the scoring player.
void handle_score(Player *p1, Player *p2, Ball *b)
{
    // if player 1 scores, increment score, reset ball position
    if (b->rect.x > p2->rect.x + p2->rect.w)
    {
        p1->score.val += 1;
        update_score_texture(&p1->score);
        b->rect.x = SCREEN_WIDTH / 2;
        b->rect.y = SCREEN_HEIGHT / 2;
        b->x_vel = -5;
        b->y_vel = 0;
    }
    // if player 2 scores, increment score, reset ball position
    if (b->rect.x < p1->rect.x)
    {
        p2->score.val += 1;
        update_score_texture(&p2->score);
        b->rect.x = SCREEN_WIDTH / 2;
        b->rect.y = SCREEN_HEIGHT / 2;
        b->x_vel = 5;
        b->y_vel = 0;
    }
}

void update_score_texture(Score *score)
{
    if (score->texture == NULL)
    {
        puts("Score texture was unexpectedly null");
    }
    // Extract dimensions of current score
    int x_pos = score->rect.x;
    int y_pos = score->rect.y;
    int width = score->rect.w;
    int height = score->rect.h;

    // Destory old score texture
    SDL_DestroyTexture(score->texture);
    score->texture = NULL;

    SDL_Texture *texture = NULL;
    if (!create_score_texture(&texture, score->color, score->val, &width, &height))
    {
        puts("Failed to create score texture");
    }
    score->texture = texture;
}

void close_sdl(Ball *b, Player *p1, Player *p2)
{
    // Free loaded images
    SDL_DestroyTexture(b->texture);
    SDL_DestroyTexture(p1->score.texture);
    SDL_DestroyTexture(p2->score.texture);

    //Free audio files
    Mix_FreeChunk(p1->sound);
    Mix_FreeChunk(p2->sound);

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
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}

int get_y_pos(int y_pos, int vel, int player_height)
{
    int new_pos = y_pos + vel;
    // Only update position if in bounds of screen
    if (new_pos >= 0 && new_pos <= SCREEN_HEIGHT - player_height)
    {
        return new_pos;
    }
    return y_pos;
}

void update_ball(Ball *b, Player p1, Player p2)
{
    int max_speed = 27.0f;
    // Collision with player 1
    if (p1.rect.x + p1.rect.w >= b->rect.x && b->rect.y + b->rect.h >= p1.rect.y &&
        b->rect.y - b->rect.h <= p1.rect.y + p1.rect.h)
    {
        b->y_vel = calculate_y_velocity(b->rect, p1.rect);
        if (fabs(b->x_vel) < max_speed)
        {
            b->x_vel *= 1.05f;
        }
        b->x_vel = fabs(b->x_vel);
        // sound effect
        Mix_PlayChannel(-1, p1.sound, 0);
    }
    // Collision with player 2
    else if (b->rect.x >= p2.rect.x - p2.rect.w &&
             b->rect.y + b->rect.h >= p2.rect.y &&
             b->rect.y - b->rect.h <= p2.rect.y + p2.rect.h)
    {
        b->y_vel = calculate_y_velocity(b->rect, p2.rect);
        if (fabs(b->x_vel) < max_speed)
        {
            b->x_vel *= 1.05;
        }
        b->x_vel = -fabs(b->x_vel);
        // sound effect
        Mix_PlayChannel(-1, p2.sound, 0);
    }
    // Collision with top
    else if (b->rect.y <= 0)
    {
        b->y_vel = fabs(b->y_vel);
    }
    // Collision with bottom
    else if (b->rect.y + b->rect.h >= SCREEN_HEIGHT)
    {
        b->y_vel = -fabs(b->y_vel);
    }

    // Update position
    b->rect.x += b->x_vel;
    b->rect.y += b->y_vel;
}

float calculate_y_velocity(SDL_Rect ball, SDL_Rect player)
{
    int mid_paddle = player.y + player.h / 2;
    int mid_ball = ball.y + ball.h / 2;
    int multiplier = 5;
    float y_vel = (float)(mid_ball - mid_paddle) / (player.h / 2);
    return y_vel * multiplier;
}

int get_new_pos(int current, float velocity)
{
    return current + velocity;
}

void draw(Ball ball, Player p1, Player p2)
{
    // Clear Screen
    SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(g_renderer);

    // Draw player 1 red
    SDL_SetRenderDrawColor(g_renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(g_renderer, &p1.rect);
    // Draw Player 1 score
    SDL_RenderCopy(g_renderer, p1.score.texture, NULL, &p1.score.rect);

    // Draw player 2 blue
    SDL_SetRenderDrawColor(g_renderer, 0x00, 0x00, 0xFF, 0xFF);
    SDL_RenderFillRect(g_renderer, &p2.rect);
    // Draw player 2 score
    SDL_RenderCopy(g_renderer, p2.score.texture, NULL, &p2.score.rect);

    // Draw ball
    SDL_RenderCopy(g_renderer, ball.texture, NULL, &ball.rect);

    SDL_RenderPresent(g_renderer);
}