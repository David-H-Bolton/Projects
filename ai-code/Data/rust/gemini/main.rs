use csv::{ReaderBuilder, WriterBuilder};
use serde::{Deserialize, Serialize};
use std::collections::BTreeSet;
use std::error::Error;
use std::fs::File;
use std::io::{self, BufReader, BufWriter};

// Struct to represent a row in the input CSV
#[derive(Debug, Deserialize)]
struct InputRow {
    #[serde(rename = "Value1")]
    value1: f64,
    #[serde(rename = "time1")]
    time1: f64,
    #[serde(rename = "Value2")]
    value2: f64,
    #[serde(rename = "time2")]
    time2: f64,
}

// Struct to represent a row in the output CSV
#[derive(Debug, Serialize)]
struct OutputRow {
    #[serde(rename = "Time")]
    time: f64,
    #[serde(rename = "Value1")]
    value1: f64,
    #[serde(rename = "Trade1Start")]
    trade1_start: u8,
    #[serde(rename = "Value2")]
    value2: f64,
    #[serde(rename = "Trade2Start")]
    trade2_start: u8,
}

fn main() -> Result<(), Box<dyn Error>> {
    // Define input and output file paths
    let input_file_path = "data.csv";
    let output_file_path = "output.csv";

    println!("Starting trade analysis...");
    println!("Reading from: {}", input_file_path);

    // Read the input CSV file
    let file = File::open(input_file_path)?;
    let reader = ReaderBuilder::new().has_headers(true).from_reader(BufReader::new(file));

    let mut raw_data: Vec<InputRow> = Vec::new();
    for result in reader.into_deserialize() {
        let record: InputRow = result?;
        raw_data.push(record);
    }

    if raw_data.is_empty() {
        eprintln!("Input CSV file is empty. Creating an empty output file.");
        let output_file = File::create(output_file_path)?;
        let mut writer = WriterBuilder::new().has_headers(true).from_writer(BufWriter::new(output_file));
        writer.write_record(&["Time", "Value1", "Trade1Start", "Value2", "Trade2Start"])?;
        writer.flush()?;
        return Ok(());
    }

    // Process raw data to identify trade starts and create event streams
    let mut events1: Vec<(f64, f64, u8)> = Vec::new(); // (time, value, trade_start)
    let mut events2: Vec<(f64, f64, u8)> = Vec::new(); // (time, value, trade_start)
    let mut master_times_set: BTreeSet<f64> = BTreeSet::new();

    let mut prev_value1: Option<f64> = None;
    let mut prev_value2: Option<f64> = None;

    for row in raw_data {
        // Determine Trade1Start
        let trade1_start = match prev_value1 {
            Some(prev) if prev != row.value1 => 1,
            None => 1, // First value is considered a trade start
            _ => 0,
        };
        events1.push((row.time1, row.value1, trade1_start));
        master_times_set.insert(row.time1);
        prev_value1 = Some(row.value1);

        // Determine Trade2Start
        let trade2_start = match prev_value2 {
            Some(prev) if prev != row.value2 => 1,
            None => 1, // First value is considered a trade start
            _ => 0,
        };
        events2.push((row.time2, row.value2, trade2_start));
        master_times_set.insert(row.time2);
        prev_value2 = Some(row.value2);
    }

    // Sort event streams by time (important for the merging logic)
    events1.sort_by(|a, b| a.0.partial_cmp(&b.0).unwrap());
    events2.sort_by(|a, b| a.0.partial_cmp(&b.0).unwrap());

    // Create a sorted vector of all unique times
    let master_times: Vec<f64> = master_times_set.into_iter().collect();

    // Generate synchronized output rows
    let mut output_rows: Vec<OutputRow> = Vec::new();
    let mut idx1 = 0;
    let mut idx2 = 0;

    // Initialize current values with the very first available data point if any
    let mut current_value1 = events1.get(0).map_or(0.0, |e| e.1);
    let mut current_value2 = events2.get(0).map_or(0.0, |e| e.1);

    for &current_output_time in &master_times {
        let mut new_trade1_start = 0;
        let mut new_trade2_start = 0;

        // Update Value1 and Trade1Start
        // Iterate through events1 until we find events whose time is greater than current_output_time
        // The last event processed will give us the current_value1 and its trade_start status for this time point.
        while idx1 < events1.len() && events1[idx1].0 <= current_output_time {
            current_value1 = events1[idx1].1;
            new_trade1_start = events1[idx1].2; // This will be 1 if a trade started at this exact time, 0 otherwise
            idx1 += 1;
        }

        // Update Value2 and Trade2Start
        while idx2 < events2.len() && events2[idx2].0 <= current_output_time {
            current_value2 = events2[idx2].1;
            new_trade2_start = events2[idx2].2; // This will be 1 if a trade started at this exact time, 0 otherwise
            idx2 += 1;
        }

        // Add the synchronized row to the output
        output_rows.push(OutputRow {
            time: current_output_time,
            value1: current_value1,
            trade1_start: new_trade1_start,
            value2: current_value2,
            trade2_start: new_trade2_start,
        });
    }

    // Write the output CSV file
    println!("Writing to: {}", output_file_path);
    let output_file = File::create(output_file_path)?;
    let mut writer = WriterBuilder::new().has_headers(true).from_writer(BufWriter::new(output_file));

    for record in output_rows {
        writer.serialize(record)?;
    }
    writer.flush()?;

    println!("Analysis complete. Output written to {}.", output_file_path);

    Ok(())
}
