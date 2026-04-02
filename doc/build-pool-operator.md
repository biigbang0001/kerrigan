# Building Kerrigan for Pool Operators

Quick-start guide for building `kerrigand` (headless daemon) from source on common Linux distributions. Optimized for pool operators who need a working daemon -- no GUI, no wallet, no tests.

## Supported Platforms

| Distro | GCC | Status | Notes |
|--------|-----|--------|-------|
| Ubuntu 24.04 | 13.2 | Native | All deps in repos |
| Ubuntu 22.04 | 11.4 | Native | Needs `libbacktrace` from depends |
| Ubuntu 20.04 | 9.4 | **Needs PPA** | GCC 9 too old (need 11+), use `ppa:ubuntu-toolchain-r/test` |
| Debian 12 | 12.2 | Native | Needs `libbacktrace` from depends |
| Debian 11 | 10.2 | **Needs backports** | GCC 10 too old, install `gcc-12` from backports |
| Fedora 38+ | 13+ | Native | No `libbacktrace` package, use `--disable-stacktraces` |
| Rocky/Alma 9 | 11.4 | Native | Via `gcc-toolset-12` |
| Arch Linux | Latest | Native | All deps in repos |

## Requirements

- **GCC 11.1+** or **Clang 16+** (C++20 support mandatory)
- **Rust 1.81+** (for Sapling zk-SNARK cryptography)
- **cxxbridge 1.0.186** (exact version -- C++/Rust FFI bridge)
- ~1.5 GB RAM for compilation (or tune with `--param ggc-min-expand=1`)
- ~4 GB disk for source + build

## One-Liner Install (Ubuntu 22.04/24.04)

```bash
# Install everything and build in one shot
sudo apt update && \
sudo apt install -y build-essential libtool autotools-dev automake pkg-config \
  bsdmainutils bison python3 libevent-dev libboost-dev libsqlite3-dev \
  libgmp-dev libsodium-dev libzmq3-dev libminiupnpc-dev libnatpmp-dev curl && \
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y && \
source "$HOME/.cargo/env" && \
cargo install cxxbridge-cmd --version 1.0.186 && \
git clone https://github.com/kerrigan-network/kerrigan.git && \
cd kerrigan && \
./autogen.sh && \
./configure --without-gui --disable-wallet --disable-tests --disable-bench && \
make -j$(nproc)
```

## Step-by-Step Instructions

### System Dependencies

#### Ubuntu 24.04 (Noble)
```bash
sudo apt update
sudo apt install -y build-essential libtool autotools-dev automake pkg-config \
  bsdmainutils bison python3 libevent-dev libboost-dev libsqlite3-dev \
  libgmp-dev libsodium-dev libzmq3-dev libminiupnpc-dev libnatpmp-dev \
  libbacktrace-dev
```

#### Ubuntu 22.04 (Jammy)
```bash
sudo apt update
sudo apt install -y build-essential libtool autotools-dev automake pkg-config \
  bsdmainutils bison python3 libevent-dev libboost-dev libsqlite3-dev \
  libgmp-dev libsodium-dev libzmq3-dev libminiupnpc-dev libnatpmp-dev

# libbacktrace not available -- will be built from depends or use --disable-stacktraces
```

#### Ubuntu 20.04 (Focal) -- Needs newer GCC
```bash
# Add toolchain PPA for GCC 11+
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install -y gcc-12 g++-12 build-essential libtool autotools-dev automake \
  pkg-config bsdmainutils bison python3 libevent-dev libboost-dev libsqlite3-dev \
  libgmp-dev libsodium-dev libzmq3-dev libminiupnpc-dev libnatpmp-dev

# Set GCC 12 as default
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100
```

#### Debian 12 (Bookworm)
```bash
sudo apt update
sudo apt install -y build-essential libtool autotools-dev automake pkg-config \
  bsdmainutils bison python3 libevent-dev libboost-dev libsqlite3-dev \
  libgmp-dev libsodium-dev libzmq3-dev libminiupnpc-dev libnatpmp-dev
```

#### Debian 11 (Bullseye) -- Needs newer GCC
```bash
# Enable backports
echo "deb http://deb.debian.org/debian bullseye-backports main" | sudo tee /etc/apt/sources.list.d/backports.list
sudo apt update
sudo apt install -y -t bullseye-backports gcc-12 g++-12
sudo apt install -y build-essential libtool autotools-dev automake pkg-config \
  bsdmainutils bison python3 libevent-dev libboost-dev libsqlite3-dev \
  libgmp-dev libsodium-dev libzmq3-dev libminiupnpc-dev libnatpmp-dev

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100
```

#### Fedora 38+
```bash
sudo dnf install -y gcc-c++ libtool make autoconf automake python3 \
  libevent-devel boost-devel sqlite-devel gmp-devel libsodium-devel \
  zeromq-devel miniupnpc-devel libnatpmp-devel
```

#### Rocky Linux / AlmaLinux 9
```bash
sudo dnf install -y gcc-toolset-12-gcc-c++ libtool make autoconf automake python3 \
  libevent-devel boost-devel sqlite-devel gmp-devel libsodium-devel \
  zeromq-devel miniupnpc-devel

# Enable the toolset for this session
scl enable gcc-toolset-12 bash
```

### Rust and cxxbridge

**This step is required on ALL platforms.** Distro Rust packages are usually too old.

```bash
# Install Rust via rustup (official installer)
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
source "$HOME/.cargo/env"

# Verify version (need 1.81+)
rustc --version

# Install the exact cxxbridge version required
cargo install cxxbridge-cmd --version 1.0.186

# Verify
cxxbridge --version
# Should output: cxxbridge 1.0.186
```

### Clone and Build

```bash
git clone https://github.com/kerrigan-network/kerrigan.git
cd kerrigan
./autogen.sh

# Pool operator build: no GUI, no wallet, no tests
./configure \
  --without-gui \
  --disable-wallet \
  --disable-tests \
  --disable-bench \
  --disable-stacktraces  # add this if libbacktrace is not available

make -j$(nproc)

# Binary is at src/kerrigand
ls -la src/kerrigand
```

### Install (Optional)

```bash
sudo make install
# Installs to /usr/local/bin/kerrigand by default
```

## Build Using Depends (Alternative -- Most Portable)

If you have trouble with system libraries, the `depends/` system builds everything from source:

```bash
# Install minimal build tools only
sudo apt install -y build-essential libtool autotools-dev automake pkg-config \
  bsdmainutils bison python3 curl cmake

# Install Rust + cxxbridge (still required)
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
source "$HOME/.cargo/env"
cargo install cxxbridge-cmd --version 1.0.186

# Build all dependencies from source (~20-30 minutes)
cd kerrigan
make -C depends -j$(nproc) NO_QT=1 NO_WALLET=1

# Configure with depends
export CONFIG_SITE=$PWD/depends/$(depends/config.guess)/share/config.site
./autogen.sh
./configure --without-gui --disable-wallet --disable-tests --disable-bench
make -j$(nproc)
```

## Common Build Errors

### `cargo not found`
```
configure: error: cargo not found. Install Rust via https://rustup.rs/
```
**Fix**: Install Rust per the "Rust and cxxbridge" section above, then `source ~/.cargo/env`.

### `cxxbridge not found`
```
configure: error: cxxbridge not found. Install via: cargo install cxxbridge-cmd --version 1.0.186
```
**Fix**: `cargo install cxxbridge-cmd --version 1.0.186`

### `C++20 support not found`
```
configure: error: *** A compiler with support for C++20 language features is required.
```
**Fix**: Your GCC is too old. Need GCC 11+ or Clang 16+. See platform-specific instructions above.

### `libbacktrace not found`
```
configure: error: libbacktrace is required
```
**Fix**: Either install `libbacktrace-dev` (Ubuntu 25.04+/Debian 13+) or add `--disable-stacktraces` to configure.

### `libsodium not found`
```
checking for crypto_generichash_blake2b_init in -lsodium... no
```
**Fix**: `sudo apt install libsodium-dev` (Debian/Ubuntu) or `sudo dnf install libsodium-devel` (Fedora).

### `libgmp not found`
```
checking for __gmpz_init in -lgmp... no
```
**Fix**: `sudo apt install libgmp-dev` (Debian/Ubuntu) or `sudo dnf install gmp-devel` (Fedora).

### Rust compilation fails with memory errors
```
error: could not compile `bellman`
```
**Fix**: Rust compilation (especially Sapling's bellman/groth16) needs ~2GB RAM. On low-memory VPS, add swap:
```bash
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

### `make` fails with parallelism errors
```
error: *** No rule to make target 'src/rust/gen/bridge.rs.cc'
```
**Fix**: Run `make` without `-j` first (single-threaded) to let Cargo generate the bridge files, then re-run with `-j$(nproc)`:
```bash
make -j1 && make clean && make -j$(nproc)
# Or just: make -j$(nproc)  (usually works, retry without -j if it doesn't)
```

## Pool Daemon Configuration

After building, see [pool-integration.md](pool-integration.md) for:
- `rpcalgoport` configuration (per-algo RPC ports)
- `pooladdress` setup (coinbase recipient)
- Miningcore / S-NOMP integration
- Troubleshooting mining issues

## Verifying Your Build

```bash
# Check version
./src/kerrigand --version

# Quick test: start on mainnet, sync headers
./src/kerrigand -printtoconsole -datadir=/tmp/kerrigan-test

# Test RPC
./src/kerrigan-cli -rpcuser=pool -rpcpassword=test getblockchaininfo
```
