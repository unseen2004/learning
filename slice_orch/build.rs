fn main() {
    println!("cargo:rerun-if-changed=proto/slice.proto");
    println!("cargo:rerun-if-changed=proto/telemetry.proto");
    println!("cargo:rerun-if-changed=proto/policy.proto");

    // Set vendored protoc
    if let Ok(p) = protoc_bin_vendored::protoc_bin_path() {
        std::env::set_var("PROTOC", &p);
    }

    // Perform code generation for gRPC services.
    if let Err(e) = tonic_build::configure()
        .build_server(true)
        .build_client(true)
        .compile_protos(
            &[
                "proto/slice.proto",
                "proto/telemetry.proto",
                "proto/policy.proto",
            ],
            &["proto"],
        ) {
        eprintln!("build.rs: failed to compile protos: {e}");
        std::process::exit(1);
    }
}
