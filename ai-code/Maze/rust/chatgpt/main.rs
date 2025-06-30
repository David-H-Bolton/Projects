use rand::seq::SliceRandom;
use rand::thread_rng;
use std::fs::File;
use std::io::{self, Write};

const WIDTH: usize = 21; // Must be odd
const HEIGHT: usize = 21; // Must be odd

fn main() -> io::Result<()> {
    let mut maze = [['*'; WIDTH]; HEIGHT];

    // Generate the maze starting from (1,1)
    carve_maze(1, 1, &mut maze);

    // Create entry and exit
    maze[0][1] = ' '; // Entry at top
    maze[HEIGHT - 1][WIDTH - 2] = ' '; // Exit at bottom

    // Write maze to file
    let mut file = File::create("maze.txt")?;
    for row in maze.iter() {
        for &cell in row.iter() {
            write!(file, "{}", cell)?;
        }
        writeln!(file)?;
    }

    println!("Maze written to maze.txt");
    Ok(())
}

fn carve_maze(x: usize, y: usize, maze: &mut [[char; WIDTH]; HEIGHT]) {
    maze[y][x] = ' ';

    let mut directions = vec![(2, 0), (-2, 0), (0, 2), (0, -2)];
    directions.shuffle(&mut thread_rng());

    for (dx, dy) in directions {
        let nx = x as isize + dx;
        let ny = y as isize + dy;

        if nx > 0 && nx < WIDTH as isize - 1 && ny > 0 && ny < HEIGHT as isize - 1 {
            if maze[ny as usize][nx as usize] == '*' {
                maze[(y as isize + dy / 2) as usize][(x as isize + dx / 2) as usize] = ' ';
                carve_maze(nx as usize, ny as usize, maze);
            }
        }
    }
}
