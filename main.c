#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int ROWS = 8;
int COLS = 16;

typedef enum { DEAD = 0, ALIVE } States;

#define CELL(g, y, x) g[(y * COLS + x)]

int *front_buf; // last
int *back_buf;  // next

void init_grid(void);
void deinit_grid(void);

void display(void);
void step(void);
int get_neboures(int center_y, int center_x);
void randomize_grid(void);

int main(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, ":w:h:r")) != -1) {
        switch (opt) {
        case 'w': printf("option: %c value: %s\n", opt, optarg); break;
        case 'h': printf("option: %c value: %s\n", opt, optarg); break;
        case 'r': printf("option: %c\n", opt); break;
        case ':': printf("option: -%c needs value\n", opt); break;
        case '?': printf("unknown option: %c\n", optopt); break;
        }
    }

    // Glider:
    // front_buf[0][1] = 1;
    // front_buf[1][2] = 1;
    // front_buf[2][0] = 1;
    // front_buf[2][1] = 1;
    // front_buf[2][2] = 1;

    // sidways L
    // front_buf[0][0] = 1;
    // front_buf[0][1] = 1;
    // front_buf[0][2] = 1;
    // front_buf[1][0] = 1;

    // Line
    // front_buf[2][3] = 1;
    // front_buf[3][3] = 1;
    // front_buf[4][3] = 1;

    init_grid();
    randomize_grid();

    for (;;) {
        display();
        step();
        memcpy(front_buf, back_buf, sizeof(int) * ROWS * COLS);
        printf("\033[%dA\033[%dD", ROWS, COLS);
        usleep(100 * 1000);
    }

    deinit_grid();
    return 0;
}

void init_grid(void)
{
    front_buf = malloc(ROWS * COLS * sizeof(int));
    back_buf = malloc(ROWS * COLS * sizeof(int));
}

void deinit_grid(void)
{
    free(front_buf);
    free(back_buf);
}

void display(void)
{
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (CELL(front_buf, y, x) == ALIVE)
                printf("#");
            else
                printf(".");
        }
        printf("\n");
    }
}

void step(void)
{
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            int neboures = get_neboures(y, x);

            if (CELL(front_buf, y, x) == ALIVE) {
                CELL(back_buf, y, x) = (neboures == 2 || neboures == 3);
            } else {
                CELL(back_buf, y, x) = (neboures == 3);
            }
        }
    }
}
int mod(int a, int b) { return (a % b + b) % b; }

int get_neboures(int center_y, int center_x)
{
    // -1, -1 | -1, 0  | -1, 1
    // 0, -1  |  0, 0  | 0, 1
    // 1, -1  |  1, 0  | 1, 1

    int neboures = 0;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (!(dy == 0 && dx == 0)) {
                int x = mod(center_x + dx, COLS);
                int y = mod(center_y + dy, ROWS);

                if (CELL(front_buf, y, x) == ALIVE) neboures += 1;
            }
        }
    }
    return neboures;
}

void randomize_grid(void)
{
    srand(time(NULL));

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (rand() % 100 >= 50) {
                CELL(front_buf, y, x) = ALIVE;
            }
        }
    }
}
