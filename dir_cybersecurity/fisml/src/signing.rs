use ed25519_dalek::{SigningKey, VerifyingKey, Signature, Signer, Verifier};
use rand::rngs::OsRng;
use crate::errors::{Result, FismlError};
use std::path::Path;
use std::fs;

#[derive(Clone)]
pub struct KeyPair {
    pub signing: SigningKey,
    pub verifying: VerifyingKey,
}

impl KeyPair {
    pub fn load_or_generate(path: &Path) -> Result<Self> {
        if path.exists() {
            let bytes = fs::read(path)?;
            if bytes.len() != 32 { return Err(FismlError::Crypto("Invalid key length".into())); }
            let signing = SigningKey::from_bytes(&bytes.try_into().unwrap());
            Ok(Self { verifying: signing.verifying_key(), signing })
        } else {
            let mut rng = OsRng;
            let signing = SigningKey::generate(&mut rng);
            fs::write(path, signing.to_bytes())?;
            Ok(Self { verifying: signing.verifying_key(), signing })
        }
    }

    pub fn sign(&self, data: &[u8]) -> Vec<u8> {
        self.signing.sign(data).to_bytes().to_vec()
    }

    pub fn verify(&self, data: &[u8], sig: &[u8]) -> bool {
        if let Ok(signature) = Signature::from_slice(sig) {
            self.verifying.verify(data, &signature).is_ok()
        } else { false }
    }

    pub fn sign_hash(&self, hash_hex: &str) -> Vec<u8> {
        self.signing.sign(hash_hex.as_bytes()).to_bytes().to_vec()
    }

    pub fn verify_hash(&self, hash_hex: &str, sig: &[u8]) -> bool {
        if let Ok(signature) = Signature::from_slice(sig) {
            self.verifying.verify(hash_hex.as_bytes(), &signature).is_ok()
        } else { false }
    }
}
