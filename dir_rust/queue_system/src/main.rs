use anyhow::Result;
use clap::{Parser, Subcommand};
use crossterm::{
    execute,
    terminal::{Clear, ClearType},
};
use std::io::stdout;

mod queue;
mod qr;
mod scanner;
mod ui;

use queue::QueueManager;

#[derive(Parser)]
#[command(name = "queue_system")]
#[command(about = "QR Code Queue Management System", long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    Generate,
    Scan,
    Status,
    Run,
}

#[tokio::main]
async fn main() -> Result<()> {
    let cli = Cli::parse();

    match cli.command {
        Commands::Generate => {
            let id = qr::generate_qr()?;
            println!("Generated QR code with ID: {}", id);
            println!("QR code saved as: qr_{}.png", id);
        }
        Commands::Scan => {
            println!("Starting camera for QR scanning...");
            scanner::scan_and_add().await?;
        }
        Commands::Status => {
            let manager = QueueManager::new();
            ui::display_status(&manager)?;
        }
        Commands::Run => {
            run_interactive().await?;
        }
    }

    Ok(())
}

async fn run_interactive() -> Result<()> {
    let mut manager = QueueManager::new();
    let mut stdout = stdout();

    loop {
        execute!(stdout, Clear(ClearType::All))?;
        ui::display_full_ui(&manager)?;

        println!("\n[1] Generate QR  [2] Scan QR  [3] Refresh  [4] Exit");
        println!("Enter choice: ");

        let mut input = String::new();
        std::io::stdin().read_line(&mut input)?;

        match input.trim() {
            "1" => {
                let id = qr::generate_qr()?;
                println!("\nGenerated QR with ID: {}", id);
                println!("Press Enter to continue...");
                let mut _pause = String::new();
                std::io::stdin().read_line(&mut _pause)?;
            }
            "2" => {
                println!("\nStarting scanner...");
                if let Ok(id) = scanner::scan_qr_code() {
                    manager.process_scan(&id).await?;
                    println!("Press Enter to continue...");
                    let mut _pause = String::new();
                    std::io::stdin().read_line(&mut _pause)?;
                }
            }
            "3" => {
                continue;
            }
            "4" => {
                println!("Exiting...");
                break;
            }
            _ => {
                println!("Invalid choice");
                tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
            }
        }
    }

    Ok(())
}
