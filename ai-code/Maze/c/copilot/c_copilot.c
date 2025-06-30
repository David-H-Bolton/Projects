#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 21   // Must be odd
#define HEIGHT 21  // Must be odd

char maze[HEIGHT][WIDTH];

// Direction vectors: Up, Down, Left, Right
int dx[] = { 0, 0, -2, 2 };
int dy[] = { -2, 2, 0, 0 };

void initMaze() {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            maze[y][x] = '*';  // fill with walls
}

int isValid(int x, int y) {
    return x > 0 && x < WIDTH - 1 && y > 0 && y < HEIGHT - 1;
}

void carve(int x, int y) {
    maze[y][x] = ' ';
    int dirs[] = { 0, 1, 2, 3 };

    // Shuffle directions
    for (int i = 0; i < 4; i++) {
        int r = rand() % 4;
        int temp = dirs[i];
        dirs[i] = dirs[r];
        dirs[r] = temp;
    }

    for (int i = 0; i < 4; i++) {
        int nx = x + dx[dirs[i]];
        int ny = y + dy[dirs[i]];
        if (isValid(nx, ny) && maze[ny][nx] == '*') {
            maze[y + dy[dirs[i]] / 2][x + dx[dirs[i]] / 2] = ' ';
            carve(nx, ny);
        }
    }
}

void saveMaze(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Failed to open file");
        exit(1);
    }
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++)
            fputc(maze[y][x], f);
        fputc('\n', f);
    }
    fclose(f);
}

int main() {
    srand(time(NULL));

    initMaze();
    carve(1, 1);  // Start carving from (1,1)

    maze[0][1] = ' ';             // Entry
    maze[HEIGHT - 1][WIDTH - 2] = ' ';  // Exit

    saveMaze("maze.txt");

    printf("Maze saved to maze.txt\n");
    return 0;
}
