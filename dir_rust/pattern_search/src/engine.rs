use anyhow::{Context, Result};
use log::{debug, warn};
use serde::Serialize;
use std::fs::File;
use std::io::{self, BufRead, BufReader, Read, StdinLock};
use std::path::Path;
use crate::cli::Cli;

#[derive(Serialize)]
struct MatchLine<'a> {
    file: &'a str,
    #[serde(skip_serializing_if = "Option::is_none")]
    line: Option<usize>,
    text: &'a str,
}

#[derive(Serialize)]
struct CountLine<'a> {
    file: &'a str,
    count: usize,
}

pub fn run(cli: Cli) -> Result<i32> {
    let pattern = cli.pattern.to_lowercase();

    let mut total_matches = 0usize;
    let mut had_error = false;
    let use_stdin = cli.paths.is_empty()
        || (cli.paths.len() == 1 && cli.paths[0].as_os_str() == "-");
    if use_stdin {
        debug!("Reading from stdin");
        let stdin = io::stdin();
        let lock = stdin.lock();
        let matched = process_reader(
            "<stdin>",
            reader_from_stdin(lock),
            &pattern,
            &cli,
        )?;
        total_matches += matched;
        output_count_if_needed("<stdin>", matched, &cli);
    } else {
        for path in &cli.paths {
            if path.as_os_str() == "-" {
                debug!("Reading from stdin (mixed with file list)");
                let stdin = io::stdin();
                let lock = stdin.lock();
                let matched = process_reader(
                    "<stdin>",
                    reader_from_stdin(lock),
                    &pattern,
                    &cli,
                )?;
                total_matches += matched;
                output_count_if_needed("<stdin>", matched, &cli);
                continue;
            }
            match File::open(path) {
                Ok(file) => {
                    debug!("Opened file: {:?}", path);
                    let matched = process_reader(
                        display_path(path),
                        BufReader::new(file),
                        &pattern,
                        &cli,
                    )?;
                    total_matches += matched;
                    output_count_if_needed(display_path(path), matched, &cli);
                }
                Err(e) => {
                    warn!("failed to open '{}': {}", path.display(), e);
                    had_error = true;
                }
            }
        }
    }
    if had_error {
        return Ok(exitcode::IOERR);
    }
    if total_matches == 0 {
        return Ok(1);
    }
    Ok(exitcode::OK)
}

fn process_reader<R: BufRead>(
    label: &str,
    mut reader: R,
    pattern: &str,
    cli: &Cli,
) -> Result<usize> {
    let mut buf = String::new();
    let mut matches = 0usize;
    let mut line_no = 0usize;
    loop {
        buf.clear();
        let bytes = reader.read_line(&mut buf).context("failed to read line")?;
        if bytes == 0 {
            break;
        }
        let mut line = buf.as_str();
        if line.ends_with('\n') {
            line = &line[..line.len() - 1];
        }
        line_no += 1;

        let haystack_matches = line.to_lowercase().starts_with(pattern);

        let is_match = if cli.invert_match { !haystack_matches } else { haystack_matches };
        if is_match {
            matches += 1;
            if !cli.count {
                print_line(label, line_no, line, cli);
            }
        }
    }
    Ok(matches)
}

fn print_line(label: &str, line_no: usize, line: &str, cli: &Cli) {
    if cli.json {
        let rec = MatchLine {
            file: label,
            line: if cli.line_number { Some(line_no) } else { None },
            text: line,
        };
        println!("{}", serde_json::to_string(&rec).unwrap());
        return;
    }
    let show_filename = cli.paths.len() > 1;
    match (show_filename, cli.line_number) {
        (true, true) => println!("{}:{}:{}", label, line_no, line),
        (true, false) => println!("{}:{}", label, line),
        (false, true) => println!("{}:{}", line_no, line),
        (false, false) => println!("{}", line),
    }
}

fn output_count_if_needed(label: &str, count: usize, cli: &Cli) {
    if !cli.count {
        return;
    }
    if cli.json {
        let rec = CountLine { file: label, count };
        println!("{}", serde_json::to_string(&rec).unwrap());
        return;
    }
    let show_filename = cli.paths.len() > 1;
    if show_filename {
        println!("{}:{}", label, count);
    } else {
        println!("{}", count);
    }
}

fn reader_from_stdin<'a>(lock: StdinLock<'a>) -> BufReader<StdinReader<'a>> {
    BufReader::new(StdinReader(lock))
}

struct StdinReader<'a>(StdinLock<'a>);
impl<'a> Read for StdinReader<'a> {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        self.0.read(buf)
    }
}

fn display_path(path: &Path) -> &str {
    path.to_str().unwrap_or("<invalid-utf8>")
}