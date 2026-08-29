FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    linux-tools-generic \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

# Build the project
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build -j
