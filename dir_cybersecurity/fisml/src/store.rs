use rusqlite::{Connection, params};
use chrono::{Utc, DateTime};
use std::path::Path;
use crate::model::{EventRecord, EventKind, FileRecord};
use crate::errors::Result;
use crate::logchain::compute_event_hash;

pub struct Store {
    conn: Connection,
}

impl Store {
    pub fn open(path: &Path) -> Result<Self> {
        let conn = Connection::open(path)?;
        let s = Self { conn };
        s.init()?;
        Ok(s)
    }

    fn init(&self) -> Result<()> {
        self.conn.execute_batch(r#"
            CREATE TABLE IF NOT EXISTS files(
                path TEXT PRIMARY KEY,
                hash TEXT NOT NULL,
                size INTEGER NOT NULL,
                mtime INTEGER NOT NULL,
                mode INTEGER NOT NULL,
                last_seen INTEGER NOT NULL
            );
            CREATE TABLE IF NOT EXISTS events(
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                ts TEXT NOT NULL,
                path TEXT NOT NULL,
                kind TEXT NOT NULL,
                old_hash TEXT,
                new_hash TEXT,
                secret_count INTEGER NOT NULL,
                prev_event_hash TEXT,
                event_hash TEXT NOT NULL,
                signature BLOB
            );
        "#)?;
        Ok(())
    }

    pub fn upsert_file(&self, rec: &FileRecord) -> Result<()> {
        self.conn.execute(r#"INSERT INTO files(path, hash, size, mtime, mode, last_seen) VALUES(?,?,?,?,?,?)
            ON CONFLICT(path) DO UPDATE SET hash=excluded.hash,size=excluded.size,mtime=excluded.mtime,mode=excluded.mode,last_seen=excluded.last_seen"#,
            params![rec.path, rec.hash, rec.size, rec.mtime, rec.mode, rec.mtime])?;
        Ok(())
    }

    pub fn get_file_hash(&self, path: &str) -> Result<Option<String>> {
        let mut stmt = self.conn.prepare("SELECT hash FROM files WHERE path=?1")?;
        let mut rows = stmt.query(params![path])?;
        if let Some(row) = rows.next()? { Ok(Some(row.get(0)?)) } else { Ok(None) }
    }

    fn last_event_hash(&self) -> Result<Option<String>> {
        let mut stmt = self.conn.prepare("SELECT event_hash FROM events ORDER BY id DESC LIMIT 1")?;
        let mut rows = stmt.query([])?;
        if let Some(r) = rows.next()? { Ok(Some(r.get(0)?)) } else { Ok(None) }
    }

    pub fn add_event(&self, mut ev: EventRecord) -> Result<EventRecord> {
        ev.prev_event_hash = self.last_event_hash()?;
        let h = compute_event_hash(&ev);
        ev.event_hash = h.clone();
        self.conn.execute(r#"INSERT INTO events(ts,path,kind,old_hash,new_hash,secret_count,prev_event_hash,event_hash,signature) VALUES(?,?,?,?,?,?,?,?,?)"#,
            params![ev.ts.to_rfc3339(), ev.path, format!("{:?}", ev.kind), ev.old_hash, ev.new_hash, ev.secret_count as i64, ev.prev_event_hash, ev.event_hash, ev.signature])?;
        ev.id = Some(self.conn.last_insert_rowid());
        Ok(ev)
    }

    pub fn list_events(&self, limit: usize) -> Result<Vec<EventRecord>> {
        let mut stmt = self.conn.prepare("SELECT id,ts,path,kind,old_hash,new_hash,secret_count,prev_event_hash,event_hash,signature FROM events ORDER BY id DESC LIMIT ?1")?;
        let rows = stmt.query_map(params![limit as i64], |row| {
            let ts_s: String = row.get(1)?;
            let ts: DateTime<Utc> = ts_s.parse().map_err(|e| rusqlite::Error::FromSqlConversionFailure(0, rusqlite::types::Type::Text, Box::new(e)))?;
            let kind_s: String = row.get(3)?;
            let kind = match kind_s.as_str() { "Baseline" => EventKind::Baseline, "New" => EventKind::New, "Modified" => EventKind::Modified, "Deleted" => EventKind::Deleted, _ => EventKind::Modified };
            Ok(EventRecord { id: row.get(0)?, ts, path: row.get(2)?, kind, old_hash: row.get(4)?, new_hash: row.get(5)?, secret_count: row.get::<_, i64>(6)? as usize, prev_event_hash: row.get(7)?, event_hash: row.get(8)?, signature: row.get(9)? })
        })?;
        let mut out = Vec::new();
        for r in rows { out.push(r?); }
        Ok(out)
    }

    pub fn all_events(&self) -> Result<Vec<EventRecord>> {
        let mut stmt = self.conn.prepare("SELECT id,ts,path,kind,old_hash,new_hash,secret_count,prev_event_hash,event_hash,signature FROM events ORDER BY id ASC")?;
        let rows = stmt.query_map([], |row| {
            let ts_s: String = row.get(1)?;
            let ts: DateTime<Utc> = ts_s.parse().map_err(|e| rusqlite::Error::FromSqlConversionFailure(0, rusqlite::types::Type::Text, Box::new(e)))?;
            let kind_s: String = row.get(3)?;
            let kind = match kind_s.as_str() { "Baseline" => EventKind::Baseline, "New" => EventKind::New, "Modified" => EventKind::Modified, "Deleted" => EventKind::Deleted, _ => EventKind::Modified };
            Ok(EventRecord { id: row.get(0)?, ts, path: row.get(2)?, kind, old_hash: row.get(4)?, new_hash: row.get(5)?, secret_count: row.get::<_, i64>(6)? as usize, prev_event_hash: row.get(7)?, event_hash: row.get(8)?, signature: row.get(9)? })
        })?;
        let mut out = Vec::new();
        for r in rows { out.push(r?); }
        Ok(out)
    }

    pub fn delete_file(&self, path: &str) -> Result<()> {
        self.conn.execute("DELETE FROM files WHERE path=?1", params![path])?;
        Ok(())
    }

    pub fn update_signature(&self, id: i64, sig: &[u8]) -> Result<()> {
        self.conn.execute("UPDATE events SET signature=?1 WHERE id=?2", params![sig, id])?;
        Ok(())
    }

    pub fn list_files(&self) -> Result<Vec<String>> {
        let mut stmt = self.conn.prepare("SELECT path FROM files")?;
        let rows = stmt.query_map([], |row| row.get(0))?;
        let mut v = Vec::new();
        for r in rows { v.push(r?); }
        Ok(v)
    }
}
