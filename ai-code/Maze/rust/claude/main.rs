use std::fs::File;
use std::io::Write;
use rand::Rng;

const WIDTH: usize = 41;  // Must be odd for proper maze generation
const HEIGHT: usize = 21; // Must be odd for proper maze generation

#[derive(Clone, Copy)]
struct Cell {
    x: usize,
    y: usize,
}

struct Maze {
    grid: [[char; WIDTH]; HEIGHT],
    visited: [[bool; WIDTH]; HEIGHT],
}

impl Maze {
    fn new() -> Self {
        let mut maze = Maze {
            grid: [['*'; WIDTH]; HEIGHT],
            visited: [[false; WIDTH]; HEIGHT],
        };
        
        // Initialize all odd positions as corridors (will be carved out during generation)
        for y in (1..HEIGHT).step_by(2) {
            for x in (1..WIDTH).step_by(2) {
                maze.grid[y][x] = ' ';
            }
        }
        
        maze
    }
    
    fn generate(&mut self) {
        let mut rng = rand::thread_rng();
        let start = Cell { x: 1, y: 1 };
        let mut stack = Vec::new();
        
        self.visited[start.y][start.x] = true;
        stack.push(start);
        
        while let Some(current) = stack.last().copied() {
            let neighbors = self.get_unvisited_neighbors(current);
            
            if !neighbors.is_empty() {
                let next = neighbors[rng.gen_range(0..neighbors.len())];
                
                // Remove wall between current and next cell
                let wall_x = (current.x + next.x) / 2;
                let wall_y = (current.y + next.y) / 2;
                self.grid[wall_y][wall_x] = ' ';
                
                self.visited[next.y][next.x] = true;
                stack.push(next);
            } else {
                stack.pop();
            }
        }
        
        // Create entry and exit points
        self.create_entry_exit();
    }
    
    fn get_unvisited_neighbors(&self, cell: Cell) -> Vec<Cell> {
        let mut neighbors = Vec::new();
        let directions = [(0, 2), (2, 0), (0, -2), (-2, 0)]; // Up, Right, Down, Left
        
        for (dx, dy) in directions.iter() {
            let new_x = cell.x as i32 + dx;
            let new_y = cell.y as i32 + dy;
            
            if new_x >= 1 && new_x < (WIDTH - 1) as i32 && 
               new_y >= 1 && new_y < (HEIGHT - 1) as i32 {
                let nx = new_x as usize;
                let ny = new_y as usize;
                
                if !self.visited[ny][nx] {
                    neighbors.push(Cell { x: nx, y: ny });
                }
            }
        }
        
        neighbors
    }
    
    fn create_entry_exit(&mut self) {
        // Entry at top-left area
        self.grid[0][1] = ' ';
        
        // Exit at bottom-right area
        self.grid[HEIGHT - 1][WIDTH - 2] = ' ';
    }
    
    fn write_to_file(&self, filename: &str) -> Result<(), std::io::Error> {
        let mut file = File::create(filename)?;
        
        for row in &self.grid {
            for &cell in row {
                write!(file, "{}", cell)?;
            }
            writeln!(file)?;
        }
        
        Ok(())
    }
    
    fn print(&self) {
        for row in &self.grid {
            for &cell in row {
                print!("{}", cell);
            }
            println!();
        }
    }
}

fn main() {
    println!("Generating maze...");
    
    let mut maze = Maze::new();
    maze.generate();
    
    // Print maze to console
    maze.print();
    
    // Write maze to file
    match maze.write_to_file("maze.txt") {
        Ok(()) => println!("\nMaze successfully written to maze.txt"),
        Err(e) => eprintln!("Error writing maze to file: {}", e),
    }
}

// Add this to Cargo.toml dependencies:
// [dependencies]
// rand = "0.8"