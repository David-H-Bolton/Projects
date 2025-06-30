#pragma warning(disable:4996);
#include <stdio.h>   // Required for file I/O (fopen, fprintf, fclose) and console output (printf)
#include <stdlib.h>  // Required for random number generation (rand, srand) and general utilities
#include <time.h>    // Required for seeding the random number generator (time)

// Define the default dimensions of the maze.
// It's crucial for these to be ODD numbers to allow for proper wall and corridor
// generation using the recursive backtracker algorithm.
// A maze of size (2N+1) x (2M+1) will have N x M "cells" for paths.
#define DEFAULT_WIDTH 41  // Example: 41 columns (20 cells + 21 walls)
#define DEFAULT_HEIGHT 21 // Example: 21 rows (10 cells + 11 walls)

// Global 2D array to store the maze structure.
// Using global arrays simplifies passing them around functions, but limits
// the maze size to the defined constants.
char maze[DEFAULT_HEIGHT][DEFAULT_WIDTH];

// Global 2D array to keep track of visited "cells" during maze generation.
// The dimensions are half of the maze dimensions because each "cell" in the
// logical maze corresponds to a 2x2 block (or similar) in the physical maze.
int visited[DEFAULT_HEIGHT / 2][DEFAULT_WIDTH / 2];

// Arrays to define movement directions for the recursive backtracker.
// dx and dy represent changes in x (row) and y (column) coordinates.
// The values are 2 because we move from one "cell" to another, skipping the wall in between.
// {0, 0, 2, -2} for dx: Right, Left, Down, Up (in terms of row change)
// {2, -2, 0, 0} for dy: Right, Left, Down, Up (in terms of column change)
int dx[] = { 0, 0, 2, -2 }; // Row changes: no change, no change, down 2, up 2
int dy[] = { 2, -2, 0, 0 }; // Column changes: right 2, left 2, no change, no change

/**
 * @brief Initializes the maze grid by filling it entirely with wall characters.
 * @param width The width of the maze.
 * @param height The height of the maze.
 */
void initializeMaze(int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            maze[i][j] = '*'; // Fill every cell with a wall
        }
    }
}

/**
 * @brief Initializes the visited array, marking all logical cells as unvisited.
 * @param cell_width The logical width of the maze in terms of cells (width / 2).
 * @param cell_height The logical height of the maze in terms of cells (height / 2).
 */
void initializeVisited(int cell_width, int cell_height) {
    for (int i = 0; i < cell_height; i++) {
        for (int j = 0; j < cell_width; j++) {
            visited[i][j] = 0; // 0 means unvisited
        }
    }
}

/**
 * @brief Generates the maze using the Recursive Backtracker (Depth-First Search) algorithm.
 * This function recursively carves paths through the maze.
 * @param x The current row coordinate in the maze grid.
 * @param y The current column coordinate in the maze grid.
 * @param width The total width of the maze.
 * @param height The total height of the maze.
 */
void generateMaze(int x, int y, int width, int height) {
    maze[x][y] = ' '; // Mark the current position as a corridor (path)
    visited[x / 2][y / 2] = 1; // Mark the corresponding logical cell as visited

    // Create an array of indices representing the four possible directions.
    int directions[] = { 0, 1, 2, 3 };

    // Shuffle the directions randomly to ensure a varied maze each time.
    for (int i = 0; i < 4; i++) {
        int r = rand() % 4; // Generate a random index
        // Swap current direction with a random one
        int temp = directions[i];
        directions[i] = directions[r];
        directions[r] = temp;
    }

    // Iterate through the shuffled directions
    for (int i = 0; i < 4; i++) {
        int dir_idx = directions[i]; // Get the current direction index

        // Calculate the coordinates of the next "cell" (2 steps away from current)
        int nx = x + dx[dir_idx];
        int ny = y + dy[dir_idx];

        // Calculate the coordinates of the wall segment between the current and next cell (1 step away)
        int wx = x + dx[dir_idx] / 2;
        int wy = y + dy[dir_idx] / 2;

        // Check if the next cell is within the maze boundaries and has not been visited yet.
        // The bounds check ensures we don't go out of array limits.
        // The visited check prevents revisiting already carved paths, ensuring a single path.
        if (nx >= 0 && nx < height && ny >= 0 && ny < width && visited[nx / 2][ny / 2] == 0) {
            maze[wx][wy] = ' '; // Carve out the wall to create a path
            generateMaze(nx, ny, width, height); // Recursively call for the next cell
        }
    }
}

/**
 * @brief Sets a single entry point and a single exit point for the maze.
 * The entry is on the top row, and the exit is on the bottom row.
 * Both points are chosen randomly and ensured to be corridors.
 * @param width The width of the maze.
 * @param height The height of the maze.
 */
void setEntryExit(int width, int height) {
    // Set Entry point:
    // Choose a random odd column index for the entry on the top row (row 0).
    // (width - 2) / 2 gives the number of possible odd columns.
    // Multiplying by 2 and adding 1 ensures an odd index.
    int entry_col = (rand() % ((width - 2) / 2)) * 2 + 1;
    maze[0][entry_col] = ' '; // Make the entry point a corridor

    // Set Exit point:
    // Choose a random odd column index for the exit on the bottom row (height - 1).
    int exit_col = (rand() % ((width - 2) / 2)) * 2 + 1;
    maze[height - 1][exit_col] = ' '; // Make the exit point a corridor
}

/**
 * @brief Writes the generated maze to a text file named "maze.txt".
 * @param width The width of the maze.
 * @param height The height of the maze.
 */
void writeMazeToFile(int width, int height) {
    FILE* fp; // File pointer
    fp = fopen("maze.txt", "w"); // Open maze.txt in write mode ("w")

    // Check if the file was opened successfully
    if (fp == NULL) {
        printf("Error: Could not open maze.txt for writing!\n");
        return; // Exit if file cannot be opened
    }

    // Iterate through the maze array and write each character to the file
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fprintf(fp, "%c", maze[i][j]); // Write character
        }
        fprintf(fp, "\n"); // Write a newline character at the end of each row
    }

    fclose(fp); // Close the file
    printf("Maze successfully written to maze.txt\n");
}

/**
 * @brief Main function where the program execution begins.
 * @return 0 if the program executes successfully.
 */
int main() {
    // Seed the random number generator using the current time.
    // This ensures a different maze is generated each time the program runs.
    srand(time(NULL));

    // Use the default dimensions for the maze.
    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;

    // Ensure that the maze dimensions are odd.
    // This is critical for the recursive backtracker algorithm to work correctly
    // by ensuring that cells and walls align properly.
    if (width % 2 == 0) width++;
    if (height % 2 == 0) height++;

    // Initialize the maze with all walls and mark all logical cells as unvisited.
    initializeMaze(width, height);
    initializeVisited(width / 2, height / 2);

    // Start the maze generation process from an arbitrary starting point.
    // We start at (1,1) because (0,0) is typically a wall, and we need an odd coordinate
    // to represent the center of a "cell" for the algorithm.
    generateMaze(1, 1, width, height);

    // After the maze structure is generated, set the entry and exit points.
    setEntryExit(width, height);

    // Finally, write the complete maze to the specified text file.
    writeMazeToFile(width, height);

    return 0; // Indicate successful execution
}
