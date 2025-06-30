use std::fs::File;
use std::io::{self, Write};
use rand::seq::SliceRandom;
use rand::Rng;

// Define maze dimensions. Both width and height MUST be odd numbers
// to ensure the recursive backtracking algorithm works correctly
// by carving paths between cells.
const MAZE_WIDTH: usize = 21;
const MAZE_HEIGHT: usize = 21;

// Define the characters used for maze components.
const WALL: char = '*';
const PATH: char = ' ';

fn main() -> io::Result<()> {
    // Initialize the maze grid with all walls.
    // We create a 2D vector (Vec of Vecs) filled with WALL characters.
    let mut maze = vec![vec![WALL; MAZE_WIDTH]; MAZE_HEIGHT];

    // Obtain a thread-local random number generator.
    let mut rng = rand::thread_rng();

    // Determine a random starting point for the maze generation.
    // The starting coordinates must be odd to align with the grid structure
    // where paths are carved between odd-indexed cells.
    // `| 1` bitwise OR ensures the number is odd.
    let start_x = rng.gen_range(1..(MAZE_WIDTH - 1)) | 1;
    let start_y = rng.gen_range(1..(MAZE_HEIGHT - 1)) | 1;

    // Start the recursive backtracking process to carve paths in the maze.
    carve_path(&mut maze, start_x, start_y, &mut rng);

    // Set the entry point for the maze.
    // It's placed on the top row at a random odd column.
    let entry_col = rng.gen_range(1..(MAZE_WIDTH - 1)) | 1;
    maze[0][entry_col] = PATH;

    // Set the exit point for the maze.
    // It's placed on the bottom row at a random odd column.
    let exit_col = rng.gen_range(1..(MAZE_WIDTH - 1)) | 1;
    maze[MAZE_HEIGHT - 1][exit_col] = PATH;

    // Create a new file named "maze.txt" (or overwrite if it exists).
    let mut file = File::create("maze.txt")?;

    // Write the generated maze to the file, row by row.
    for row in maze.iter() {
        // Convert each row (Vec<char>) into a String for easy writing.
        let line: String = row.iter().collect();
        // Write the line to the file, followed by a newline character.
        writeln!(file, "{}", line)?;
    }

    println!("Maze generated and saved to maze.txt");

    Ok(()) // Indicate successful execution.
}

/// Recursively carves paths through the maze using a randomized depth-first search (recursive backtracking).
///
/// `maze`: A mutable reference to the 2D vector representing the maze grid.
/// `x`, `y`: The current coordinates (column, row) in the maze being processed.
/// `rng`: A mutable reference to the random number generator.
fn carve_path(maze: &mut Vec<Vec<char>>, x: usize, y: usize, rng: &mut impl Rng) {
    // Mark the current cell as a path.
    maze[y][x] = PATH;

    // Define possible directions to move: (dx, dy) for (change in x, change in y).
    // Each move is 2 units to ensure we land on another "cell" and can carve a wall in between.
    let mut directions = vec![(0, -2), (0, 2), (-2, 0), (2, 0)]; // Up, Down, Left, Right

    // Randomize the order of directions to ensure a varied maze structure.
    directions.shuffle(rng);

    // Iterate through each randomized direction.
    for &(dx, dy) in directions.iter() {
        // Calculate the coordinates of the next potential cell.
        // Using `isize` for intermediate calculations to handle negative results from subtraction
        // before casting back to `usize` for array indexing.
        let nx = x as isize + dx;
        let ny = y as isize + dy;

        // Check if the next potential cell is within the maze boundaries.
        if nx >= 0 && nx < MAZE_WIDTH as isize && ny >= 0 && ny < MAZE_HEIGHT as isize {
            let next_x = nx as usize;
            let next_y = ny as usize;

            // If the next cell is currently a wall, it means it hasn't been visited yet.
            if maze[next_y][next_x] == WALL {
                // Carve a path between the current cell and the next cell.
                // This involves changing the character of the wall cell directly between them.
                maze[(y as isize + dy / 2) as usize][(x as isize + dx / 2) as usize] = PATH;

                // Recursively call `carve_path` for the newly opened cell.
                carve_path(maze, next_x, next_y, rng);
            }
        }
    }
}
