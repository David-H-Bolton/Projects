use std::collections::BTreeMap;
use std::error::Error;
use std::fs::File;
use std::io::Write;

use csv::{ReaderBuilder, WriterBuilder};
use serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize)]
struct InputRow {
    value1: f64,
    time1: f64,
    value2: f64,
    time2: f64,
}

#[derive(Debug, Serialize)]
struct OutputRow {
    time: f64,
    value1: Option<f64>,
    value2: Option<f64>,
    trade1_started: u8,
    trade2_started: u8,
}

fn main() -> Result<(), Box<dyn Error>> {
    let mut rdr = ReaderBuilder::new()
        .has_headers(true)
        .from_path("data.csv")?;

    let mut output: BTreeMap<f64, OutputRow> = BTreeMap::new();

    let mut prev1: Option<f64> = None;
    let mut prev2: Option<f64> = None;

    for result in rdr.deserialize() {
        let row: InputRow = result?;

        // Handle first trade stream
        let trade1_started = match prev1 {
            Some(prev) if prev != row.value1 => 1,
            None => 1,
            _ => 0,
        };
        prev1 = Some(row.value1);
        output
            .entry(row.time1)
            .and_modify(|entry| {
                entry.value1 = Some(row.value1);
                entry.trade1_started = trade1_started;
            })
            .or_insert(OutputRow {
                time: row.time1,
                value1: Some(row.value1),
                value2: None,
                trade1_started,
                trade2_started: 0,
            });

        // Handle second trade stream
        let trade2_started = match prev2 {
            Some(prev) if prev != row.value2 => 1,
            None => 1,
            _ => 0,
        };
        prev2 = Some(row.value2);
        output
            .entry(row.time2)
            .and_modify(|entry| {
                entry.value2 = Some(row.value2);
                entry.trade2_started = trade2_started;
            })
            .or_insert(OutputRow {
                time: row.time2,
                value1: None,
                value2: Some(row.value2),
                trade1_started: 0,
                trade2_started,
            });
    }

    let mut wtr = WriterBuilder::new().from_path("output.csv")?;
    wtr.write_record(&["time", "value1", "value2", "trade1_started", "trade2_started"])?;

    for (_, row) in output {
        wtr.serialize(row)?;
    }

    wtr.flush()?;
    println!("✅ Trades synchronized and written to output.csv");

    Ok(())
}
