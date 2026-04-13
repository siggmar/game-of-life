#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h>

int grid_rows = 8;
int grid_cols = 16;

float SIM_SPEED = 0.1;
bool randomize;

typedef enum { DEAD = 0, ALIVE } States;

#define CELL(b, y, x) b[(y * grid_cols + x)]

int *front_buf; // last
int *back_buf;  // next

void init_grid(void);
void deinit_grid(void);
void reset_grid(void);

void display(void);
void display_stats(void);
void step(void);
int get_neboures(int center_y, int center_x);
void randomize_grid(void);

int parse_args(int argc, char *argv[]);

void handle_input(int ch);

void resize_buf();

int main(int argc, char *argv[])
{
    int result = parse_args(argc, argv);
    if (result != 0) {
        return result;
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

    initscr();
    noecho();
    nodelay(stdscr, TRUE); // no getch delay
    keypad(stdscr, TRUE);
    curs_set(0);

    init_grid();

    if (randomize) randomize_grid();

    int ch;

    while ((ch = getch()) != 'q') {
        handle_input(ch);
        flushinp(); // flush ch buffer, usleep will cause bugs without it
        display();
        display_stats();
        step();
        memcpy(front_buf, back_buf, sizeof(int) * grid_rows * grid_cols);

        timeout(SIM_SPEED * 1000);

        // usleep(1000 * SIM_SPEED * 1000);
    }

    deinit_grid();
    endwin();

    return 0;
}

void init_grid(void)
{
    front_buf = malloc(grid_rows * grid_cols * sizeof(int));
    back_buf = malloc(grid_rows * grid_cols * sizeof(int));
}

void deinit_grid(void)
{
    free(front_buf);
    free(back_buf);
}

void reset_grid(void)
{
    memset(front_buf, 0, grid_rows * grid_cols * sizeof(int));
    memset(back_buf, 0, grid_rows * grid_cols * sizeof(int));
}

void display(void)
{
    erase();
    for (int y = 0; y < grid_rows; y++) {
        for (int x = 0; x < grid_cols; x++) {
            if (CELL(front_buf, y, x) == ALIVE)
                mvaddch(y, x, '#');
            else
                mvaddch(y, x, '.');
        }
    }
    refresh();
}

void step(void)
{
    for (int y = 0; y < grid_rows; y++) {
        for (int x = 0; x < grid_cols; x++) {
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
                int x = mod(center_x + dx, grid_cols);
                int y = mod(center_y + dy, grid_rows);

                if (CELL(front_buf, y, x) == ALIVE) neboures += 1;
            }
        }
    }
    return neboures;
}

void randomize_grid(void)
{
    srand(time(NULL));

    for (int y = 0; y < grid_rows; y++) {
        for (int x = 0; x < grid_cols; x++) {
            if (rand() % 100 >= 50) {
                CELL(front_buf, y, x) = ALIVE;
            }
        }
    }
}

int parse_args(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, ":r:c:s:gh")) != -1) {
        switch (opt) {
        case 'r':
            grid_rows = strtol(optarg, NULL, 10);
            if (grid_rows <= 0) {
                fprintf(stderr, "Invalid number of rows: %s\n", optarg);
                return -1;
            }
            break;
        case 'c':
            grid_cols = strtol(optarg, NULL, 10);
            if (grid_cols <= 0) {
                fprintf(stderr, "Invalid number of COLS: %s\n", optarg);
                return -1;
            }

            break;
        case 's':
            SIM_SPEED = strtod(optarg, NULL);
            if (SIM_SPEED <= 0) {
                fprintf(stderr, "Invalid simulation speed: %s\n", optarg);
                return -1;
            }

            break;
        case 'g': randomize = true; break;
        case 'h':
            printf(
                "usage: main.c [-h] [-r <n>] \n\n"
                "optional arguments: \n\t "
                "-h,\tshow this help message and exit \n\t "
                "-r <optarg>,\tchange rows to <optarg> \n\t "
                "-c <optarg>,\tchange cols to <optarg> \n\t "
                "-s <optarg>,\tchange simulation speed to <optarg> seconds "
                "\n\t "
                "-g,\trandomize grid \n"
            );
            return -1;
        case ':': printf("option -%c requires value\n", optopt); return -1;
        case '?': printf("unknown option: -%c\n", optopt); return -1;
        }
    }
    return 0;
}

void handle_input(int ch)
{
    switch (ch) {
    case '+': SIM_SPEED -= 0.1; break;
    case '-': SIM_SPEED += 0.1; break;
    case 'r': reset_grid(); break;
    case 'R': randomize_grid(); break;

    case KEY_UP:
        grid_rows -= 1;
        resize_buf();
        printw("pressed UP \n");
        break;
    case KEY_DOWN:
        grid_rows += 1;
        resize_buf();
        break;
    case KEY_LEFT:
        grid_cols -= 1;
        resize_buf();
        break;
    case KEY_RIGHT:
        grid_cols += 1;
        resize_buf();
        break;
    }
}

void resize_buf()
{
    front_buf = realloc(front_buf, grid_rows * grid_cols * sizeof(int));
    back_buf = realloc(back_buf, grid_rows * grid_cols * sizeof(int));

    assert(front_buf != NULL);
    assert(back_buf != NULL);
}

void display_stats(void)
{
    int stats_space = 10;
    mvprintw(0, grid_cols + stats_space, "Simulation Speed: %f", SIM_SPEED);
    mvprintw(1, grid_cols + stats_space, "Grid Rows: %d", grid_rows);
    mvprintw(2, grid_cols + stats_space, "Grid Cols: %d", grid_cols);

    refresh();
}
