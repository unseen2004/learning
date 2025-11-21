use crate::queue::QueueManager;
use anyhow::Result;

pub fn display_status(manager: &QueueManager) -> Result<()> {
    println!("\n=== Queue Status ===\n");

    if let Some(first) = manager.get_first() {
        println!("FIRST IN QUEUE: {}", first.id);
        if let Some(remaining) = manager.get_time_remaining() {
            println!("Time remaining: {}s\n", remaining);
        }
    } else {
        println!("Queue is empty\n");
    }

    let queue = manager.get_queue();
    if !queue.is_empty() {
        println!("Current Queue:");
        for (idx, entry) in queue.iter().enumerate() {
            println!("{}. {}", idx + 1, entry.id);
        }
    }

    Ok(())
}

pub fn display_full_ui(manager: &QueueManager) -> Result<()> {
    println!("========================================");
    println!("      QR CODE QUEUE SYSTEM");
    println!("========================================\n");

    if let Some(first) = manager.get_first() {
        println!(">>> FIRST IN QUEUE <<<");
        println!("ID: {}", first.id);
        if let Some(remaining) = manager.get_time_remaining() {
            println!("Auto-remove in: {}s", remaining);
        }
        println!();
    } else {
        println!("Queue is empty\n");
    }

    let queue = manager.get_queue();
    if queue.len() > 1 {
        println!("Waiting in queue:");
        for (idx, entry) in queue.iter().skip(1).enumerate() {
            println!("  {}. {}", idx + 2, entry.id);
        }
        println!();
    }

    println!("Total in queue: {}", queue.len());
    println!();

    Ok(())
}
