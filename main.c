#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#define HEIGHT 10
#define WIDTH 15

/*
    Tiles:
    0 = Unexplored
    1 = Explored, blank
    2 = Pit
    3 = Bat (x2)
    4 = Wumpus
    99 = Player
*/

#define UNEXPLORED 0
#define BLANK 1
#define PIT 2
#define BAT 3
#define WUMPUS 4
#define PLAYER 99

#define BAT_COLOR "\e[30;100m"
#define WUMPUS_BKG_COLOR "\e[48;2;222;184;135m"
#define WUMPUS_COLOR "\e[38;2;139;69;19m"
#define PIT_COLOR "\e[38;2;128;255;128m"
#define BORDER_COLOR "\e[94m"
#define LIGHT_RED "\e[91m"
#define COLOR_RESET "\e[0m"

struct point {
    int x;
    int y;
};

void draw_board(int board[HEIGHT][WIDTH]);

char get_map_tile(int i);

void get_map_color(int i);

int calculate_new_tile(int board[HEIGHT][WIDTH], int x, int y);

void wrap(struct point *p);

void get_random_point(struct point *p);

void get_surrounding(struct point p);

int distance(struct point p1, struct point p2);

void shoot(int directionx, int directiony);

bool game_over = false;

struct point player;
struct point bat1, bat2;
struct point wumpus;
struct point pit1, pit2;

char status_text[100] = {0};
int board[HEIGHT][WIDTH];

int arrows = 5;
bool shooting_mode = false;

int main()
{

    srand(time(NULL));

    player.x = 0;
    player.y = 0;

    get_random_point(&bat1);
    get_random_point(&bat2);
    get_random_point(&wumpus);
    get_random_point(&pit1);
    get_random_point(&pit2);

    strcpy(status_text, "Status: ");

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            board[i][j] = UNEXPLORED;
        }
    }

    board[0][0] = BLANK;

    /*
    board[bat1.y][bat1.x] = BAT;
    board[bat2.y][bat2.x] = BAT;
    board[wumpus.y][wumpus.x] = WUMPUS;
    board[pit1.y][pit1.x] = PIT;
    board[pit2.y][pit2.x] = PIT;
    */
    //board[bat1.y][bat1.x] = BAT;

    while (!game_over) {
        //system("clear");
        draw_board(board);
        get_surrounding(player);
        printf("%s\n", status_text);
        if (game_over) {
            break;
        }
        system ("/bin/stty raw");

        if (getchar() == '\033') {
            getchar();
            switch(getchar()) {
                case 'A':
                    if (shooting_mode) {
                        shoot(0, -1);
                    } else {
                        player.y -= 1;
                    }
                    break;
                case 'B':
                    if (shooting_mode) {
                        shoot(0, 1);
                    } else {
                        player.y += 1;
                    }
                    break;
                case 'C':
                    if (shooting_mode) {
                        shoot(1, 0);
                    } else {
                        player.x += 1;
                    }
                    break;
                case 'D':
                    if (shooting_mode) {
                        shoot(-1, 0);
                    } else {
                        player.x -= 1;
                    }
                    break;
            }
            wrap(&player);
            board[player.y][player.x] = calculate_new_tile(board, player.x, player.y);
        } else if (getchar() == 'S' || getchar() == 's') {
            printf("working");
            if (arrows == 0) {
                strcpy(status_text, "You don't have any arrows.");
                continue;
            } else {
                strcpy(status_text, "Which way?");
                continue;
                //shooting_mode = true;
            }
        }

        system ("/bin/stty cooked");
    }
    printf("\n\n\nGAME OVER\n");
    return 0;
}

void wrap(struct point *p) {
    if (p->x > WIDTH - 1) {
        p->x = 0;
    } else if (p->x < 0) {
        p->x = WIDTH - 1;
    }

    if (p->y > HEIGHT - 1) {
        p->y = 0;
    } else if (p->y < 0) {
        p->y = HEIGHT - 1;
    }
}

void get_random_point(struct point *p) {
    p->x = (rand()%(WIDTH-2));
    p->y = (rand()%(HEIGHT-2));
}


int calculate_new_tile(int board[HEIGHT][WIDTH], int x, int y) {
    // do checks to see if the current playerx and y is
    // a wumpus, bat, pit, etc
    // if none of these are true, just unhide the tile
    if (board[y][x] == UNEXPLORED) {
        if ((y == bat1.y && x == bat1.x) || (y == bat2.y && x == bat2.x)) {
            return BAT;
        } else if (y == wumpus.y && x == wumpus.x) {
            return WUMPUS;
        } else if ((y == pit1.y && x == pit1.x) || (y == pit2.y && x == pit2.x)) {
            return PIT;
        } else {
            return BLANK;
        }
    }
}

void draw_board(int board[HEIGHT][WIDTH]) {
    printf(BORDER_COLOR);
    printf("████████████████████████████████\n" COLOR_RESET);
    for (int i = 0; i < HEIGHT; i++) {
        printf(BORDER_COLOR "█" COLOR_RESET);
        for (int j = 0; j < WIDTH; j++) {
            if (i == player.y && j == player.x) {
                printf("* ");
            } else {
                get_map_color(board[i][j]);
                printf("%c", get_map_tile(board[i][j]));
                printf(COLOR_RESET);
                printf(" ");
            }
        }
        printf(BORDER_COLOR "█" COLOR_RESET);
        printf("\n");
    }
    printf(BORDER_COLOR "████████████████████████████████\n" COLOR_RESET);
}

void get_surrounding(struct point p) {
    int wumpus_distance = distance(p, wumpus);
    int bat1_distance = distance(p, bat1);
    int bat2_distance = distance(p, bat2);
    int pit1_distance = distance(p, pit1);
    int pit2_distance = distance(p, pit2);

    if (wumpus_distance == 1) {
        strcpy(status_text, "Status: I smell a Wumpus...");
    } else if (pit1_distance == 1 || pit2_distance == 1) {
        strcpy(status_text, "Status: I feel a draft...");
    } else if (bat1_distance == 1 || bat2_distance == 1) {
        strcpy(status_text, "Status: I hear flapping...");
    } else {
        strcpy(status_text, "Status:");
    }

    if (wumpus_distance == 0) {
        strcpy(status_text, LIGHT_RED "CHOMP! The wumpus got you!\n" COLOR_RESET );
        game_over = true;
    }
    if (pit1_distance == 0 || pit2_distance == 0) {
        strcpy(status_text, PIT_COLOR "You fall down a pit, breaking yourself on the way down.\n" COLOR_RESET);
        game_over = true;
    }
    if (bat1_distance == 0 || bat2_distance == 0) {
        player.x = (rand()%(WIDTH-2));
        player.y = (rand()%(HEIGHT-2));
        strcpy(status_text, "Bats carried you away!");
    }

    /*
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int newx = p.x + i;
            int newy = p.y + j;
            if (calculate_new_tile(board, wrap_int(newx, 15), wrap_int(newy, 10)) == WUMPUS) {
                strcpy(status_text, "Status: Wumpus nearby");
            }

        }
    }
    */
}

int distance(struct point p1, struct point p2) {
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

void get_map_color(int i) {
    switch (i) {
    case BAT:
        printf(BAT_COLOR);
        break;
    case WUMPUS:
        printf(WUMPUS_BKG_COLOR);
        printf(WUMPUS_COLOR);
        break;
    case PIT:
        printf(PIT_COLOR);
        break;
    }
}

char temp = '0';
char get_map_tile(int i) {
    switch (i) {
    case UNEXPLORED:
        temp = '-';
        break;
    case BLANK:
        temp = ' ';
        break;
    case PIT:
        temp = 'O';
        break;
    case BAT:
        temp = 'M';
        break;
    case WUMPUS:
        temp = 'W';
        break;
    case PLAYER:
        temp = '*';
        break;
    }
    return temp;
}

void shoot (int directionx, int directiony) {
    shooting_mode = false;
    printf("%d %d", directionx, directiony);
    if (player.x + directionx == wumpus.x && player.y + directiony == wumpus.y) {
        strcpy(status_text, "You got the Wumpus!");
    }
}
