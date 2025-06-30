#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 41
#define HEIGHT 21

// Directions: North, East, South, West
int dx[] = { 0, 1, 0, -1 };
int dy[] = { -1, 0, 1, 0 };

void initializeMaze(char maze[HEIGHT][WIDTH]) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            maze[i][j] = '*';
        }
    }
}

int isValid(int x, int y, char maze[HEIGHT][WIDTH]) {
    return (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && maze[y][x] == '*');
}

int countNeighbors(int x, int y, char maze[HEIGHT][WIDTH]) {
    int count = 0;
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i] * 2;
        int ny = y + dy[i] * 2;
        if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT && maze[ny][nx] == ' ') {
            count++;
        }
    }
    return count;
}

void generateMaze(char maze[HEIGHT][WIDTH], int x, int y) {
    maze[y][x] = ' ';

    // Create array of directions and shuffle them
    int directions[4] = { 0, 1, 2, 3 };
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = directions[i];
        directions[i] = directions[j];
        directions[j] = temp;
    }

    for (int i = 0; i < 4; i++) {
        int dir = directions[i];
        int nx = x + dx[dir] * 2;
        int ny = y + dy[dir] * 2;

        if (isValid(nx, ny, maze) && countNeighbors(nx, ny, maze) == 0) {
            // Carve the wall between current cell and next cell
            maze[y + dy[dir]][x + dx[dir]] = ' ';
            generateMaze(maze, nx, ny);
        }
    }
}

void createEntryExit(char maze[HEIGHT][WIDTH]) {
    // Entry at top-left corner area
    maze[0][1] = ' ';

    // Exit at bottom-right corner area
    maze[HEIGHT - 1][WIDTH - 2] = ' ';
}

void writeMazeToFile(char maze[HEIGHT][WIDTH], const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not create file %s\n", filename);
        return;
    }

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            fputc(maze[i][j], file);
        }
        fputc('\n', file);
    }

    fclose(file);
    printf("Maze successfully written to %s\n", filename);
}

void printMaze(char maze[HEIGHT][WIDTH]) {
    printf("\nGenerated Maze:\n");
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            printf("%c", maze[i][j]);
        }
        printf("\n");
    }
}

int main() {
    char maze[HEIGHT][WIDTH];

    // Seed random number generator
    srand(time(NULL));

    // Initialize maze with all walls
    initializeMaze(maze);

    // Generate maze starting from position (1,1)
    generateMaze(maze, 1, 1);

    // Create entry and exit points
    createEntryExit(maze);

    // Print maze to console
    printMaze(maze);

    // Write maze to file
    writeMazeToFile(maze, "maze.txt");

    return 0;
}