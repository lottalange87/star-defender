#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>

#define WIDTH 60
#define HEIGHT 24
#define MAX_BULLETS 20
#define MAX_ENEMIES 15
#define MAX_PARTICLES 50

typedef struct {
    int x, y;
    int active;
} Entity;

typedef struct {
    int x, y;
    int active;
    int type; // 0 = normal, 1 = fast, 2 = tank
    int hp;
} Enemy;

typedef struct {
    int x, y;
    int active;
    int life;
    int dx, dy;
} Particle;

typedef struct {
    int x, y;
    int score;
    int lives;
    int weapon_level;
} Player;

// Game state
Player player;
Entity bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
Particle particles[MAX_PARTICLES];
int frame = 0;
int enemy_spawn_timer = 0;
int game_over = 0;
int wave = 1;

// Terminal handling
struct termios original_term;

void setup_terminal() {
    tcgetattr(STDIN_FILENO, &original_term);
    struct termios raw = original_term;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    
    // Hide cursor
    printf("\033[?25l");
    fflush(stdout);
}

void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
    printf("\033[?25h");
    fflush(stdout);
}

int kbhit() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

char getch() {
    char c = 0;
    read(STDIN_FILENO, &c, 1);
    return c;
}

void clear_screen() {
    printf("\033[2J\033[H");
}

void move_cursor(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}

void set_color(int fg, int bg) {
    printf("\033[%d;%dm", fg + 30, bg + 40);
}

void reset_color() {
    printf("\033[0m");
}

void draw_char(int x, int y, char c, int color) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        move_cursor(x, y);
        set_color(color, 0);
        putchar(c);
        reset_color();
    }
}

void draw_string(int x, int y, const char* str, int color) {
    move_cursor(x, y);
    set_color(color, 0);
    printf("%s", str);
    reset_color();
}

// Particle effects
void spawn_particle(int x, int y, int dx, int dy, int life) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].x = x;
            particles[i].y = y;
            particles[i].dx = dx;
            particles[i].dy = dy;
            particles[i].life = life;
            particles[i].active = 1;
            break;
        }
    }
}

void spawn_explosion(int x, int y, int intensity) {
    for (int i = 0; i < intensity; i++) {
        int dx = (rand() % 5) - 2;
        int dy = (rand() % 5) - 2;
        spawn_particle(x, y, dx, dy, 10 + rand() % 10);
    }
}

void update_particles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].dx;
            particles[i].y += particles[i].dy;
            particles[i].life--;
            if (particles[i].life <= 0 || 
                particles[i].x < 0 || particles[i].x >= WIDTH ||
                particles[i].y < 0 || particles[i].y >= HEIGHT) {
                particles[i].active = 0;
            }
        }
    }
}

void draw_particles() {
    const char* chars = "*+.#@";
    int colors[] = {3, 1, 1, 4, 7}; // yellow, red, red, blue, white
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            int idx = rand() % 5;
            draw_char(particles[i].x, particles[i].y, chars[idx], colors[idx]);
        }
    }
}

// Game logic
void init_game() {
    memset(bullets, 0, sizeof(bullets));
    memset(enemies, 0, sizeof(enemies));
    memset(particles, 0, sizeof(particles));
    
    player.x = WIDTH / 2;
    player.y = HEIGHT - 3;
    player.score = 0;
    player.lives = 3;
    player.weapon_level = 1;
    
    frame = 0;
    enemy_spawn_timer = 0;
    game_over = 0;
    wave = 1;
}

void fire_bullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player.x;
            bullets[i].y = player.y - 1;
            bullets[i].active = 1;
            
            // Multi-shot for higher weapon levels
            if (player.weapon_level >= 2) {
                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (!bullets[j].active && j != i) {
                        bullets[j].x = player.x - 2;
                        bullets[j].y = player.y;
                        bullets[j].active = 1;
                        break;
                    }
                }
            }
            if (player.weapon_level >= 3) {
                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (!bullets[j].active) {
                        bullets[j].x = player.x + 2;
                        bullets[j].y = player.y;
                        bullets[j].active = 1;
                        break;
                    }
                }
            }
            break;
        }
    }
}

void spawn_enemy() {
    int spawn_rate = 40 - (wave * 3);
    if (spawn_rate < 10) spawn_rate = 10;
    
    if (enemy_spawn_timer++ > spawn_rate) {
        enemy_spawn_timer = 0;
        
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) {
                enemies[i].x = rand() % (WIDTH - 4) + 2;
                enemies[i].y = 1;
                enemies[i].active = 1;
                
                // Random enemy type based on wave
                int roll = rand() % 100;
                if (wave >= 3 && roll < 15) {
                    enemies[i].type = 2; // Tank
                    enemies[i].hp = 3;
                } else if (wave >= 2 && roll < 40) {
                    enemies[i].type = 1; // Fast
                    enemies[i].hp = 1;
                } else {
                    enemies[i].type = 0; // Normal
                    enemies[i].hp = 1;
                }
                break;
            }
        }
    }
}

void update_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= 2;
            if (bullets[i].y < 0) {
                bullets[i].active = 0;
            }
        }
    }
}

void update_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            int speed = 1;
            if (enemies[i].type == 1) speed = 2;
            if (enemies[i].type == 2) speed = 1;
            
            // Zigzag movement for some enemies
            if (enemies[i].type == 1 && frame % 4 == 0) {
                enemies[i].x += (rand() % 3) - 1;
            }
            
            enemies[i].y += speed;
            
            // Check if enemy reached bottom
            if (enemies[i].y >= HEIGHT - 1) {
                enemies[i].active = 0;
                player.lives--;
                spawn_explosion(player.x, player.y, 15);
                if (player.lives <= 0) {
                    game_over = 1;
                }
            }
            
            // Keep in bounds
            if (enemies[i].x < 1) enemies[i].x = 1;
            if (enemies[i].x >= WIDTH - 1) enemies[i].x = WIDTH - 2;
        }
    }
}

void check_collisions() {
    // Bullets hitting enemies
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;
            
            // Simple collision
            if (abs(bullets[i].x - enemies[j].x) <= 1 &&
                abs(bullets[i].y - enemies[j].y) <= 1) {
                
                bullets[i].active = 0;
                enemies[j].hp--;
                
                if (enemies[j].hp <= 0) {
                    enemies[j].active = 0;
                    spawn_explosion(enemies[j].x, enemies[j].y, 8);
                    
                    // Score based on type
                    int points = 10;
                    if (enemies[j].type == 1) points = 20;
                    if (enemies[j].type == 2) points = 50;
                    player.score += points;
                    
                    // Weapon upgrade every 500 points
                    if (player.score % 500 == 0 && player.weapon_level < 3) {
                        player.weapon_level++;
                        spawn_explosion(player.x, player.y - 2, 12);
                    }
                } else {
                    spawn_explosion(enemies[j].x, enemies[j].y, 4);
                }
                break;
            }
        }
    }
    
    // Enemies hitting player
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        
        if (abs(enemies[i].x - player.x) <= 2 &&
            abs(enemies[i].y - player.y) <= 1) {
            enemies[i].active = 0;
            player.lives--;
            spawn_explosion(player.x, player.y, 20);
            
            if (player.lives <= 0) {
                game_over = 1;
            }
        }
    }
}

void draw_player() {
    const char* ship[3] = {
        " ^ ",
        "<#>",
        "/ \\"
    };
    int colors[3] = {6, 6, 6}; // cyan
    
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            int px = player.x - 1 + col;
            int py = player.y - 1 + row;
            if (ship[row][col] != ' ') {
                draw_char(px, py, ship[row][col], colors[row]);
            }
        }
    }
}

void draw_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            int color = 1; // red
            char c = 'V';
            
            if (enemies[i].type == 1) {
                color = 5; // magenta
                c = 'v';
            } else if (enemies[i].type == 2) {
                color = 3; // yellow
                c = 'W';
            }
            
            draw_char(enemies[i].x, enemies[i].y, c, color);
            if (enemies[i].type == 2 && enemies[i].hp > 1) {
                draw_char(enemies[i].x - 1, enemies[i].y, '<', color);
                draw_char(enemies[i].x + 1, enemies[i].y, '>', color);
            }
        }
    }
}

void draw_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            draw_char(bullets[i].x, bullets[i].y, '|', 3); // yellow
        }
    }
}

void draw_ui() {
    char buffer[128];
    
    // Top border
    move_cursor(0, 0);
    set_color(7, 4);
    for (int i = 0; i < WIDTH; i++) putchar('=');
    reset_color();
    
    // Stats
    snprintf(buffer, sizeof(buffer), " SCORE: %d  LIVES: %d  WAVE: %d  WEAPON: %d ", 
             player.score, player.lives, wave, player.weapon_level);
    draw_string(2, 0, buffer, 7);
    
    // Bottom border
    move_cursor(0, HEIGHT - 1);
    set_color(7, 4);
    for (int i = 0; i < WIDTH; i++) putchar('=');
    reset_color();
    
    // Controls hint
    draw_string(2, HEIGHT - 1, " WASD/Arrows: Move | SPACE: Shoot | Q: Quit ", 0);
}

void draw_game_over() {
    const char* lines[] = {
        "  GAME OVER  ",
        "             ",
        "  FINAL SCORE  ",
    };
    
    int start_y = HEIGHT / 2 - 2;
    int start_x = WIDTH / 2 - 7;
    
    for (int i = 0; i < 3; i++) {
        draw_string(start_x, start_y + i, lines[i], i == 0 ? 1 : 7);
    }
    
    char score_str[32];
    snprintf(score_str, sizeof(score_str), "    %d    ", player.score);
    draw_string(start_x, start_y + 3, score_str, 3);
    
    draw_string(start_x - 2, start_y + 5, " Press R to restart ", 2);
    draw_string(start_x - 2, start_y + 6, " Press Q to quit    ", 2);
}

void draw() {
    clear_screen();
    draw_ui();
    draw_player();
    draw_enemies();
    draw_bullets();
    draw_particles();
    
    if (game_over) {
        draw_game_over();
    }
    
    fflush(stdout);
}

void update() {
    if (game_over) return;
    
    update_bullets();
    update_enemies();
    update_particles();
    check_collisions();
    spawn_enemy();
    
    // Increase wave based on score
    wave = 1 + (player.score / 300);
    
    frame++;
}

void handle_input() {
    if (!kbhit()) return;
    
    char c = getch();
    
    if (game_over) {
        if (c == 'r' || c == 'R') {
            init_game();
        } else if (c == 'q' || c == 'Q') {
            restore_terminal();
            exit(0);
        }
        return;
    }
    
    switch (c) {
        case 'a':
            if (player.x > 2) player.x -= 2;
            break;
        case 'd':
            if (player.x < WIDTH - 3) player.x += 2;
            break;
        case 'w':
            if (player.y > 3) player.y--;
            break;
        case 's':
            if (player.y < HEIGHT - 3) player.y++;
            break;
        case ' ':
            fire_bullet();
            break;
        case 'q':
        case 'Q':
            restore_terminal();
            exit(0);
    }
    
    // Handle arrow key escape sequences
    if (c == '\033') {
        if (kbhit()) {
            char bracket = getch();
            if (bracket == '[' && kbhit()) {
                char arrow = getch();
                switch (arrow) {
                    case 'A': // Up
                        if (player.y > 3) player.y--;
                        break;
                    case 'B': // Down
                        if (player.y < HEIGHT - 3) player.y++;
                        break;
                    case 'C': // Right
                        if (player.x < WIDTH - 3) player.x += 2;
                        break;
                    case 'D': // Left
                        if (player.x > 2) player.x -= 2;
                        break;
                }
            }
        }
    }
}

int main() {
    srand(time(NULL));
    setup_terminal();
    atexit(restore_terminal);
    
    init_game();
    
    printf("\n");
    printf("    STAR DEFENDER\n");
    printf("    =============\n\n");
    printf("    Controls:\n");
    printf("    WASD or Arrows - Move\n");
    printf("    SPACE - Shoot\n");
    printf("    Q - Quit\n\n");
    printf("    Press any key to start...\n");
    
    getch();
    
    // Main game loop - target ~30 FPS
    while (1) {
        handle_input();
        update();
        draw();
        usleep(33333); // ~30 FPS
    }
    
    return 0;
}