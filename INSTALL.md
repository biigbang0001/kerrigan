# Building Kerrigan

## Quickest Path (Linux)

```bash
./build.sh
```

This detects your distro, installs dependencies (including Rust and cxxbridge), and compiles `kerrigand`. Options:

- `--with-gui` -- build the Qt wallet
- `--with-wallet` -- enable wallet support
- `--jobs N` -- parallel compile jobs (default: all cores)
- `--help` -- show all options

## Docker

```bash
docker compose up -d
```

See [contrib/containers/docker/README.md](contrib/containers/docker/README.md) for details.

## Manual Build (All Platforms)

```bash
# 1. Install system dependencies (see platform guide below)
# 2. Install Rust + cxxbridge
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
source "$HOME/.cargo/env"
cargo install cxxbridge-cmd --version 1.0.186

# 3. Build
./autogen.sh
./configure    # add --without-gui --disable-wallet for headless
make -j$(nproc)
```

## Platform Guides

- [Unix (Ubuntu, Debian, Fedora, Arch)](doc/build-unix.md)
- [macOS](doc/build-osx.md)
- [Windows](doc/build-windows.md)
- [FreeBSD](doc/build-freebsd.md)
- [Pool Operator Quick Start](doc/build-pool-operator.md)
- [Dependencies Reference](doc/dependencies.md)
