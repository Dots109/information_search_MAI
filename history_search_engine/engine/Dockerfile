FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    wget \
    git \
    gnupg \
    ca-certificates \
    python3 \
    && rm -rf /var/lib/apt/lists/*

RUN wget -qO - https://www.mongodb.org/static/pgp/server-7.0.asc | apt-key add - && \
    echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/7.0 multiverse" | tee /etc/apt/sources.list.d/mongodb-org-7.0.list

RUN apt-get update && apt-get install -y \
    libmongoc-1.0-0 \
    libmongoc-dev \
    libbson-1.0-0 \
    libbson-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp

RUN wget https://github.com/mongodb/mongo-cxx-driver/archive/refs/tags/r3.6.7.tar.gz && \
    tar xzf r3.6.7.tar.gz && \
    cd mongo-cxx-driver-r3.6.7/build && \
    cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DMONGOCXX_OVERRIDE_DEFAULT_INSTALL_PREFIX=OFF \
    -DBUILD_VERSION=3.6.7 && \
    make -j$(nproc) && make install && \
    cd /tmp && rm -rf mongo-cxx-driver-r3.6.7*

RUN ldconfig

WORKDIR /app

COPY CMakeLists.txt ./
COPY src ./src

RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

CMD ["./build/search_engine"]