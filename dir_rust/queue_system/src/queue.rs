use chrono::{DateTime, Utc};
use std::collections::VecDeque;
use std::sync::{Arc, Mutex};
use tokio::time::{sleep, Duration};

#[derive(Clone, Debug)]
pub struct QueueEntry {
    pub id: String,
    pub joined_at: DateTime<Utc>,
    pub position: usize,
}

pub struct QueueManager {
    queue: Arc<Mutex<VecDeque<QueueEntry>>>,
}

impl QueueManager {
    pub fn new() -> Self {
        Self {
            queue: Arc::new(Mutex::new(VecDeque::new())),
        }
    }

    pub async fn process_scan(&mut self, id: &str) -> anyhow::Result<()> {
        let mut queue = self.queue.lock().unwrap();

        if let Some(entry) = queue.iter().find(|e| e.id == id) {
            println!("\nAlready in queue!");
            println!("Your position: {}", entry.position);
            println!("\nCurrent queue:");
            for (idx, entry) in queue.iter().enumerate() {
                println!("{}. ID: {}", idx + 1, entry.id);
            }
        } else {
            let position = queue.len() + 1;
            let entry = QueueEntry {
                id: id.to_string(),
                joined_at: Utc::now(),
                position,
            };
            queue.push_back(entry.clone());
            println!("\nAdded to queue at position: {}", position);

            if position == 1 {
                let queue_clone = Arc::clone(&self.queue);
                let id_clone = id.to_string();
                tokio::spawn(async move {
                    sleep(Duration::from_secs(20)).await;
                    let mut q = queue_clone.lock().unwrap();
                    if let Some(front) = q.front() {
                        if front.id == id_clone {
                            q.pop_front();
                            println!("\nRemoved {} from queue (20s timeout)", id_clone);
                        }
                    }
                });
            }
        }

        Ok(())
    }

    pub fn get_queue(&self) -> Vec<QueueEntry> {
        let queue = self.queue.lock().unwrap();
        queue.iter().cloned().collect()
    }

    pub fn get_first(&self) -> Option<QueueEntry> {
        let queue = self.queue.lock().unwrap();
        queue.front().cloned()
    }

    pub fn get_time_remaining(&self) -> Option<i64> {
        let queue = self.queue.lock().unwrap();
        if let Some(first) = queue.front() {
            let elapsed = Utc::now()
                .signed_duration_since(first.joined_at)
                .num_seconds();
            let remaining = 20 - elapsed;
            Some(remaining.max(0))
        } else {
            None
        }
    }
}
