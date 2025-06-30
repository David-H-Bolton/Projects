#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 21   // Must be odd for proper maze generation
#define HEIGHT 21  // Must be odd for proper maze generation

char maze[HEIGHT][WIDTH];

// Function to initialize the maze with walls
void initializeMaze() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            maze[i][j] = '*';
        }
    }
}

// Function to shuffle directions
void shuffle(int* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int j = i + rand() / (RAND_MAX / (n - i) + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// Recursive function to carve the maze
void carveMaze(int x, int y) {
    int directions[] = { 0, 1, 2, 3 }; // Up, Right, Down, Left
    shuffle(directions, 4);

    for (int i = 0; i < 4; i++) {
        int dx = 0, dy = 0;
        switch (directions[i]) {
        case 0: dy = -1; break; // Up
        case 1: dx = 1; break;  // Right
        case 2: dy = 1; break;  // Down
        case 3: dx = -1; break; // Left
        }

        int nx = x + 2 * dx;
        int ny = y + 2 * dy;

        if (nx >= 1 && nx < WIDTH - 1 && ny >= 1 && ny < HEIGHT - 1) {
            if (maze[ny][nx] == '*') {
                maze[y + dy][x + dx] = ' ';
                maze[ny][nx] = ' ';
                carveMaze(nx, ny);
            }
        }
    }
}

void generateMaze() {
    initializeMaze();

    // Start carving from (1,1)
    maze[1][1] = ' ';
    carveMaze(1, 1);

    // Set entry and exit
    maze[0][1] = ' ';                   // Entry
    maze[HEIGHT - 1][WIDTH - 2] = ' ';  // Exit
}

void writeMazeToFile(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            fputc(maze[i][j], fp);
        }
        fputc('\n', fp);
    }

    fclose(fp);
}

int main() {
    srand(time(NULL));

    generateMaze();
    writeMazeToFile("maze.txt");

    printf("Maze generated and written to maze.txt\n");

    return 0;
}
