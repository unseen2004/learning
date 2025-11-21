use sha2::{Sha256, Digest};
#[cfg(feature = "fast-hash")] use blake3;
use std::fs::File;
use std::io::{Read, BufReader};
use anyhow::Result;

pub fn hash_bytes(data: &[u8]) -> String {
    #[cfg(feature = "fast-hash")] {
        return blake3::hash(data).to_hex().to_string();
    }
    #[allow(unreachable_code)] {
        let mut hasher = Sha256::new();
        hasher.update(data);
        format!("{:x}", hasher.finalize())
    }
}

pub fn hash_file(path: &std::path::Path) -> Result<String> {
    let f = File::open(path)?;
    let mut reader = BufReader::new(f);
    #[cfg(feature = "fast-hash")] {
        let mut hasher = blake3::Hasher::new();
        let mut buf = [0u8; 8192];
        loop {
            let n = reader.read(&mut buf)?;
            if n == 0 { break; }
            hasher.update(&buf[..n]);
        }
        return Ok(hasher.finalize().to_hex().to_string());
    }
    #[allow(unreachable_code)] {
        let mut hasher = Sha256::new();
        let mut buf = [0u8; 8192];
        loop {
            let n = reader.read(&mut buf)?;
            if n == 0 { break; }
            hasher.update(&buf[..n]);
        }
        Ok(format!("{:x}", hasher.finalize()))
    }
}

