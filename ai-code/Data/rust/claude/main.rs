use std::collections::BTreeMap;
use std::error::Error;
use std::fs::File;
use std::io::{BufReader, BufWriter};

// Add to Cargo.toml:
// [dependencies]
// csv = "1.3"
// serde = { version = "1.0", features = ["derive"] }

use csv::{Reader, Writer};
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Deserialize)]
struct InputRow {
    #[serde(rename = "Value1")]
    value1: f64,
    #[serde(rename = "time1")]
    time1: f64,
    #[serde(rename = "value2")]
    value2: f64,
    #[serde(rename = "time2")]
    time2: f64,
}

#[derive(Debug, Clone)]
struct ProcessedRow {
    value1: f64,
    time1: f64,
    value2: f64,
    time2: f64,
    trade1_start: bool,
    trade2_start: bool,
}

#[derive(Debug, Clone, Serialize)]
struct OutputRow {
    #[serde(rename = "Time")]
    time: f64,
    #[serde(rename = "Value1")]
    value1: f64,
    #[serde(rename = "Value2")]
    value2: f64,
    #[serde(rename = "Trade1_Start")]
    trade1_start: u8,
    #[serde(rename = "Trade2_Start")]
    trade2_start: u8,
}

fn read_csv_data(filename: &str) -> Result<Vec<InputRow>, Box<dyn Error>> {
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut csv_reader = Reader::from_reader(reader);
    
    let mut data = Vec::new();
    for result in csv_reader.deserialize() {
        let record: InputRow = result?;
        data.push(record);
    }
    
    Ok(data)
}

fn identify_trade_starts(data: &[InputRow]) -> Vec<ProcessedRow> {
    let mut processed_data = Vec::new();
    
    for (i, row) in data.iter().enumerate() {
        let trade1_start = if i == 0 {
            true
        } else {
            (row.value1 - data[i - 1].value1).abs() > f64::EPSILON
        };
        
        let trade2_start = if i == 0 {
            true
        } else {
            (row.value2 - data[i - 1].value2).abs() > f64::EPSILON
        };
        
        processed_data.push(ProcessedRow {
            value1: row.value1,
            time1: row.time1,
            value2: row.value2,
            time2: row.time2,
            trade1_start,
            trade2_start,
        });
    }
    
    processed_data
}

fn synchronize_trades(data: &[ProcessedRow]) -> Vec<OutputRow> {
    // Use BTreeMap to automatically sort by time
    let mut time_map: BTreeMap<String, OutputRow> = BTreeMap::new();
    
    // Process each row and create entries for both time points
    for row in data {
        // Create a key with high precision for time1
        let time1_key = format!("{:.15}", row.time1);
        let time2_key = format!("{:.15}", row.time2);
        
        // Handle time1 entry
        let entry1 = time_map.entry(time1_key.clone()).or_insert(OutputRow {
            time: row.time1,
            value1: row.value1,
            value2: 0.0,
            trade1_start: if row.trade1_start { 1 } else { 0 },
            trade2_start: 0,
        });
        
        // Update values if this entry already exists
        entry1.value1 = row.value1;
        if row.trade1_start {
            entry1.trade1_start = 1;
        }
        
        // Handle time2 entry if it's different from time1
        if (row.time2 - row.time1).abs() > f64::EPSILON {
            let entry2 = time_map.entry(time2_key).or_insert(OutputRow {
                time: row.time2,
                value1: 0.0,
                value2: row.value2,
                trade1_start: 0,
                trade2_start: if row.trade2_start { 1 } else { 0 },
            });
            
            entry2.value2 = row.value2;
            if row.trade2_start {
                entry2.trade2_start = 1;
            }
        } else {
            // Same time, so update the existing entry
            if let Some(entry) = time_map.get_mut(&time1_key) {
                entry.value2 = row.value2;
                if row.trade2_start {
                    entry.trade2_start = 1;
                }
            }
        }
    }
    
    // Convert to vector and fill missing values
    let mut synchronized_data: Vec<OutputRow> = time_map.into_values().collect();
    
    // Fill missing values by carrying forward
    let mut last_value1 = 0.0;
    let mut last_value2 = 0.0;
    
    for (i, row) in synchronized_data.iter_mut().enumerate() {
        if row.value1 != 0.0 {
            last_value1 = row.value1;
        } else {
            row.value1 = last_value1;
        }
        
        if row.value2 != 0.0 {
            last_value2 = row.value2;
        } else {
            row.value2 = last_value2;
        }
        
        // Recalculate trade starts for interpolated values
        if i > 0 && row.trade1_start == 0 {
            if (row.value1 - synchronized_data[i - 1].value1).abs() > f64::EPSILON {
                row.trade1_start = 1;
            }
        }
        
        if i > 0 && row.trade2_start == 0 {
            if (row.value2 - synchronized_data[i - 1].value2).abs() > f64::EPSILON {
                row.trade2_start = 1;
            }
        }
    }
    
    synchronized_data
}

fn write_csv_output(filename: &str, data: &[OutputRow]) -> Result<(), Box<dyn Error>> {
    let file = File::create(filename)?;
    let writer = BufWriter::new(file);
    let mut csv_writer = Writer::from_writer(writer);
    
    for row in data {
        csv_writer.serialize(row)?;
    }
    
    csv_writer.flush()?;
    Ok(())
}

fn main() -> Result<(), Box<dyn Error>> {
    println!("Reading data from data.csv...");
    
    // Read input data
    let input_data = match read_csv_data("data.csv") {
        Ok(data) => data,
        Err(e) => {
            eprintln!("Error reading data.csv: {}", e);
            eprintln!("Make sure the file exists and has columns: Value1,time1,value2,time2");
            return Err(e);
        }
    };
    
    if input_data.is_empty() {
        eprintln!("Error: No valid data found in data.csv");
        return Ok(());
    }
    
    println!("Read {} rows of data", input_data.len());
    
    // Identify trade starts
    let processed_data = identify_trade_starts(&input_data);
    
    // Synchronize trades by time
    let synchronized_data = synchronize_trades(&processed_data);
    
    println!("Generated {} synchronized rows", synchronized_data.len());
    
    // Write output
    match write_csv_output("output.csv", &synchronized_data) {
        Ok(()) => println!("Results written to output.csv"),
        Err(e) => {
            eprintln!("Error writing output.csv: {}", e);
            return Err(e);
        }
    }
    
    // Print summary statistics
    let trade1_starts = synchronized_data.iter().filter(|r| r.trade1_start == 1).count();
    let trade2_starts = synchronized_data.iter().filter(|r| r.trade2_start == 1).count();
    
    println!("Summary:");
    println!("- Total synchronized time points: {}", synchronized_data.len());
    println!("- Trade 1 starts detected: {}", trade1_starts);
    println!("- Trade 2 starts detected: {}", trade2_starts);
    
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_trade_start_identification() {
        let test_data = vec![
            InputRow { value1: 100.0, time1: 0.1, value2: 200.0, time2: 0.1 },
            InputRow { value1: 100.0, time1: 0.2, value2: 250.0, time2: 0.2 },
            InputRow { value1: 150.0, time1: 0.3, value2: 250.0, time2: 0.3 },
        ];
        
        let processed = identify_trade_starts(&test_data);
        
        assert!(processed[0].trade1_start); // First row always starts
        assert!(processed[0].trade2_start); // First row always starts
        assert!(!processed[1].trade1_start); // Same value1
        assert!(processed[1].trade2_start); // Different value2
        assert!(processed[2].trade1_start); // Different value1
        assert!(!processed[2].trade2_start); // Same value2
    }
    
    #[test]
    fn test_synchronization() {
        let test_data = vec![
            ProcessedRow {
                value1: 100.0, time1: 0.1, value2: 200.0, time2: 0.2,
                trade1_start: true, trade2_start: true,
            },
        ];
        
        let synced = synchronize_trades(&test_data);
        
        assert_eq!(synced.len(), 2); // Should create two time points
        assert_eq!(synced[0].time, 0.1);
        assert_eq!(synced[1].time, 0.2);
    }
}