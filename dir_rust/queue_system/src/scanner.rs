use anyhow::Result;
use opencv::{
    core,
    prelude::*,
    videoio,
    objdetect,
};

pub fn scan_qr_code() -> Result<String> {
    let mut cam = videoio::VideoCapture::new(0, videoio::CAP_ANY)?;
    let opened = videoio::VideoCapture::is_opened(&cam)?;
    
    if !opened {
        anyhow::bail!("Unable to open camera");
    }

    let mut qr_detector = objdetect::QRCodeDetector::default()?;

    println!("Camera opened. Show QR code to camera...");

    loop {
        let mut frame = core::Mat::default();
        cam.read(&mut frame)?;

        if frame.size()?.width > 0 {
            let mut decoded_info = String::new();
            let mut points = Mat::default();

            let detected = qr_detector.detect_and_decode(
                &frame,
                &mut decoded_info,
                &mut points,
            )?;

            if detected && !decoded_info.is_empty() {
                println!("Detected QR: {}", decoded_info);
                return Ok(decoded_info);
            }
        }
    }
}

pub async fn scan_and_add() -> Result<()> {
    let id = scan_qr_code()?;
    println!("Scanned ID: {}", id);
    Ok(())
}
