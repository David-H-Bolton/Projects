use rand::seq::SliceRandom;
use rand::thread_rng;
use std::fs::File;
use std::io::{self, Write};

const WIDTH: usize = 21;  // Must be odd
const HEIGHT: usize = 21; // Must be odd

fn main() -> io::Result<()> {
    let mut maze = vec![vec!['*'; WIDTH]; HEIGHT];
    carve_maze(1, 1, &mut maze);

    // Set entrance and exit
    maze[1][0] = ' '; // Entry
    maze[HEIGHT - 2][WIDTH - 1] = ' '; // Exit

    // Write to file
    let mut file = File::create("maze.txt")?;
    for row in &maze {
        writeln!(file, "{}", row.iter().collect::<String>())?;
    }
    println!("Maze written to maze.txt");
    Ok(())
}

fn carve_maze(x: usize, y: usize, maze: &mut Vec<Vec<char>>) {
    maze[y][x] = ' ';
    let mut dirs = vec![(2, 0), (-2, 0), (0, 2), (0, -2)];
    dirs.shuffle(&mut thread_rng());

    for (dx, dy) in dirs {
        let nx = x as isize + dx;
        let ny = y as isize + dy;

        if ny > 0 && ny < (maze.len() as isize) - 1 && nx > 0 && nx < (maze[0].len() as isize) - 1 {
            if maze[ny as usize][nx as usize] == '*' {
                maze[(y as isize + dy / 2) as usize][(x as isize + dx / 2) as usize] = ' ';
                carve_maze(nx as usize, ny as usize, maze);
            }
        }
    }
}
