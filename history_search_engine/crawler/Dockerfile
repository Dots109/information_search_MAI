FROM rust:1.83-slim

WORKDIR /app

RUN apt-get update && apt-get install -y \
    pkg-config \
    libssl-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY Cargo.toml ./
COPY config.yaml ./
COPY src ./src

RUN cargo build --release

CMD ["cargo", "run", "--release"]
