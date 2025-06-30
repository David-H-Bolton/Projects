use csv::{ReaderBuilder, WriterBuilder};
use serde::{Deserialize, Serialize};
use itertools::Itertools;
use std::error::Error;
use std::fs::File;

#[derive(Debug, Deserialize)]
struct Record {
    Value1: f64,
    time1: f64,
    Value2: f64,
    time2: f64,
}

#[derive(Debug, Serialize)]
struct OutputRecord {
    time: f64,
    Value1: Option<f64>,
    Trade1_Start: u8,
    Value2: Option<f64>,
    Trade2_Start: u8,
}

fn main() -> Result<(), Box<dyn Error>> {
    let file = File::open("data.csv")?;
    let mut rdr = ReaderBuilder::new().from_reader(file);

    let mut trade1_data = Vec::new();
    let mut trade2_data = Vec::new();

    let mut prev_val1: Option<f64> = None;
    let mut prev_val2: Option<f64> = None;

    for result in rdr.deserialize() {
        let record: Record = result?;

        let trade1_start = if Some(record.Value1) != prev_val1 {
            1
        } else {
            0
        };

        let trade2_start = if Some(record.Value2) != prev_val2 {
            1
        } else {
            0
        };

        trade1_data.push((record.time1, record.Value1, trade1_start));
        trade2_data.push((record.time2, record.Value2, trade2_start));

        prev_val1 = Some(record.Value1);
        prev_val2 = Some(record.Value2);
    }

    // Merge times and synchronize
    let mut all_times: Vec<f64> = trade1_data.iter().map(|r| r.0).collect();
    all_times.extend(trade2_data.iter().map(|r| r.0));
    all_times.sort_by(|a, b| a.partial_cmp(b).unwrap());
    all_times.dedup();

    let mut trade1_iter = trade1_data.iter().peekable();
    let mut trade2_iter = trade2_data.iter().peekable();

    let mut wtr = WriterBuilder::new().from_path("output.csv")?;

    wtr.write_record(&["time", "Value1", "Trade1_Start", "Value2", "Trade2_Start"])?;

    let mut last_val1: Option<f64> = None;
    let mut last_val2: Option<f64> = None;

    for &time in &all_times {
        let mut value1 = last_val1;
        let mut trade1_start = 0;

        if let Some(&(t, val, start)) = trade1_iter.peek() {
            if (t - time).abs() < f64::EPSILON {
                value1 = Some(val);
                trade1_start = start;
                trade1_iter.next();
            }
        }

        let mut value2 = last_val2;
        let mut trade2_start = 0;

        if let Some(&(t, val, start)) = trade2_iter.peek() {
            if (t - time).abs() < f64::EPSILON {
                value2 = Some(val);
                trade2_start = start;
                trade2_iter.next();
            }
        }

        wtr.serialize(OutputRecord {
            time,
            Value1: value1,
            Trade1_Start: trade1_start,
            Value2: value2,
            Trade2_Start: trade2_start,
        })?;

        last_val1 = value1;
        last_val2 = value2;
    }

    wtr.flush()?;
    println!("Output written to output.csv");

    Ok(())
}
