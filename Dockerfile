# ============================================================================
# AMBATRAIN C++ BACKEND MULTI-STAGE DOCKERFILE
# Cocok untuk Render.com, Railway.app, Fly.io, Koyeb, atau VPS
# ============================================================================

# Stage 1: Build Environment
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libpq-dev \
    libpqxx-dev \
    libsodium-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy CMake & Source files
COPY CMakeLists.txt .
COPY src/ ./src/

# Build C++ executable
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release -j$(nproc)

# Stage 2: Minimal Runtime Environment
FROM ubuntu:22.04 AS runner

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libpq5 \
    libpqxx-6.4 \
    libsodium23 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binary from builder stage
COPY --from=builder /app/build/ambatrain /app/ambatrain

# Environment defaults
ENV PORT=8080
ENV SERVER_HOST=0.0.0.0
EXPOSE 8080

CMD ["/app/ambatrain"]

