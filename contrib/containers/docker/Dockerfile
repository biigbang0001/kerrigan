# Multi-stage build for Kerrigan daemon
# Stage 1: Build kerrigand and kerrigan-cli from source
# Stage 2: Minimal runtime image with only shared libraries

# ---------------------------------------------------------------------------
# Build stage
# ---------------------------------------------------------------------------
FROM ubuntu:24.04 AS build

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    autoconf \
    automake \
    build-essential \
    ca-certificates \
    cmake \
    curl \
    g++ \
    git \
    libboost-dev \
    libboost-filesystem-dev \
    libboost-thread-dev \
    libevent-dev \
    libgmp-dev \
    libminiupnpc-dev \
    libnatpmp-dev \
    libsodium-dev \
    libtool \
    libzmq3-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Install Rust toolchain
ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo \
    PATH="/usr/local/cargo/bin:${PATH}"

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs \
    | sh -s -- -y --default-toolchain 1.81.0 --profile minimal \
    && rustc --version

# Install cxxbridge-cmd (must match the cxx crate version in Cargo.toml)
RUN cargo install cxxbridge-cmd --version 1.0.186 --locked

WORKDIR /src

COPY . .

RUN ./autogen.sh \
    && ./configure \
        --prefix=/usr/local \
        --without-gui \
        --disable-wallet \
        --disable-tests \
        --disable-bench \
        --disable-man \
    && make -j"$(nproc)" \
    && make install DESTDIR=/install \
    && strip /install/usr/local/bin/kerrigand \
    && strip /install/usr/local/bin/kerrigan-cli

# ---------------------------------------------------------------------------
# Runtime stage
# ---------------------------------------------------------------------------
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libevent-2.1-7 \
    libboost-filesystem1.83.0 \
    libboost-thread1.83.0 \
    libsodium23 \
    libgmp10 \
    libzmq5 \
    libminiupnpc17 \
    libnatpmp1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /install/usr/local/bin/kerrigand  /usr/local/bin/
COPY --from=build /install/usr/local/bin/kerrigan-cli /usr/local/bin/

COPY contrib/containers/docker/docker-entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

RUN groupadd --gid 1000 kerrigan \
    && useradd --uid 1000 --gid kerrigan --shell /bin/bash --create-home kerrigan \
    && mkdir -p /home/kerrigan/.kerrigan \
    && chown -R kerrigan:kerrigan /home/kerrigan

USER kerrigan

VOLUME ["/home/kerrigan/.kerrigan"]

# P2P mainnet, P2P testnet, RPC mainnet, RPC testnet
EXPOSE 7120 17120 7121 17121

WORKDIR /home/kerrigan

ENTRYPOINT ["docker-entrypoint.sh"]
CMD ["kerrigand"]
