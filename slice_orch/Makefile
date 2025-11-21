APP?=slice-orchestrator
FEATURES?=

.PHONY: build run test fmt clippy bench docker

build:
	cargo build --features "$(FEATURES)"

run:
	cargo run --features "$(FEATURES)" -- $(ARGS)

test:
	cargo test --all --features "$(FEATURES)"

fmt:
	cargo fmt --all

clippy:
	cargo clippy --all-targets --features "$(FEATURES)" -- -D warnings

bench:
	cargo bench --features bench

docker:
	docker build -t $(APP):dev .

