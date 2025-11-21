use anyhow::Result;
use image::Luma;
use qrcode::QrCode;
use uuid::Uuid;

pub fn generate_qr() -> Result<String> {
    let id = Uuid::new_v4().to_string();

    let code = QrCode::new(id.as_bytes())?;
    let image = code.render::<Luma<u8>>().build();

    let filename = format!("qr_{}.png", id);
    image.save(&filename)?;

    println!("\n{}", code.render::<char>()
        .quiet_zone(false)
        .module_dimensions(2, 1)
        .build());

    Ok(id)
}
