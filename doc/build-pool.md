Pool Operator Build Guide
=========================

Minimal instructions for building `kerrigand` and `kerrigan-cli` on a pool server.
You do not need the Qt GUI, wallet, or test suite.

## Requirements

- **OS:** Ubuntu 22.04+ or Debian 12+ (other Linux works, adjust package names)
- **RAM:** 4 GB minimum (8 GB recommended for parallel builds)
- **Disk:** ~3 GB for source + build artifacts
- **Rust:** 1.81+ via rustup (system packages are usually too old)

## 1. System Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential libtool autotools-dev automake pkg-config \
  bsdmainutils bison python3 \
  libboost-dev libevent-dev libsqlite3-dev \
  libgmp-dev libzmq3-dev libsodium-dev \
  libminiupnpc-dev libnatpmp-dev
```

**Common miss:** `libgmp-dev` is required for BLS signatures (dashbls/relic).
Without it, `./configure` fails with "libgmp headers missing."

## 2. Rust Toolchain

Kerrigan includes a Rust FFI layer for Sapling zk-SNARKs and the Hivemind
Protocol. This is the step most Dash/BTC pool operators won't expect.

```bash
# Install rustup (skip if you already have Rust 1.81+)
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
source "$HOME/.cargo/env"

# Verify version (must be >= 1.81)
rustc --version

# Install CXX bridge generator (exact version required)
cargo install cxxbridge-cmd --version 1.0.186
```

**The cxxbridge version must match exactly.** The Rust/C++ FFI bridge is pinned
to 1.0.186 in `Cargo.toml`. A version mismatch causes cryptic compile errors.

## 3. Build

```bash
./autogen.sh
./configure --without-gui --disable-tests --disable-bench
make -j$(nproc)
```

Binaries land in:
- `src/kerrigand` -- the daemon
- `src/kerrigan-cli` -- RPC command-line client

## 4. Minimal Pool Configuration

Create `~/.kerrigan/kerrigan.conf`:

```ini
# Network
server=1
daemon=1
listen=1
txindex=1

# RPC (restrict to localhost)
rpcuser=YOUR_RPC_USER
rpcpassword=YOUR_RPC_PASSWORD
rpcallowip=127.0.0.1

# Per-algo RPC ports (pool software connects to the port for its algo)
# This way getblocktemplate auto-selects the right algorithm per port.
rpcalgoport=x11:1100
rpcalgoport=kawpow:5700
rpcalgoport=equihash200:2009
rpcalgoport=equihash192:1927

# Masternode/governance (pool nodes don't need this)
# disablegovernance=1
```

### Per-Algo RPC Ports

The `-rpcalgoport` option binds additional RPC listeners, one per mining
algorithm. Pool software connects to the right port for each algo, and
`getblocktemplate` automatically returns templates for that algorithm.
No `algo` parameter needed in the RPC call.

| Algorithm      | Example Port | Config                          |
|----------------|-------------|---------------------------------|
| X11            | 1100        | `rpcalgoport=x11:1100`         |
| KawPoW         | 5700        | `rpcalgoport=kawpow:5700`      |
| Equihash 200,9 | 2009        | `rpcalgoport=equihash200:2009` |
| Equihash 192,7 | 1927        | `rpcalgoport=equihash192:1927` |

All per-algo ports bind to loopback only (127.0.0.1 / ::1). The default RPC
port (7121) continues to work normally and accepts an optional `algo` parameter
in `getblocktemplate` calls.

## 5. Start the Daemon

```bash
./src/kerrigand -conf=$HOME/.kerrigan/kerrigan.conf
```

Verify it's running:

```bash
./src/kerrigan-cli getblockchaininfo
```

Check a specific algo port:

```bash
./src/kerrigan-cli -rpcport=7123 getblocktemplate
```

## 6. Pool Software

Kerrigan is compatible with Stratum-based pool software. Each algorithm uses
standard wire formats:

| Algorithm      | Stratum Compatibility | Notes                           |
|----------------|----------------------|---------------------------------|
| X11            | Dash/DarkCoin pools  | Standard X11 stratum            |
| KawPoW         | Ravencoin pools      | RVN-compatible ProgPoW stratum  |
| Equihash 200,9 | Zcash pools          | 140-byte header, 1400-byte sol  |
| Equihash 192,7 | ZClassic pools       | Same format as 200,9            |

The Hivemind Protocol (HMP) operates entirely inside the daemon. Pool software
does not need any HMP awareness. Block templates include embedded HMP seal data
in the coinbase transaction automatically.

## Troubleshooting

### `./configure` fails with "libgmp headers missing"

```bash
sudo apt-get install libgmp-dev
```

### `./configure` fails looking for `rustc`, `cargo`, or `cxxbridge`

```bash
source "$HOME/.cargo/env"    # cargo/rustc not in PATH
cargo install cxxbridge-cmd --version 1.0.186   # cxxbridge missing or wrong version
```

### Compile error in Rust/CXX bridge code

The cxxbridge-cmd version must be exactly 1.0.186. Force reinstall:

```bash
cargo install cxxbridge-cmd --version 1.0.186 --force
make clean && make -j$(nproc)
```

### `make` runs out of memory

Reduce parallelism. Each C++ compilation unit can use 1.5 GB+ at peak:

```bash
make -j2    # instead of -j$(nproc)
```

### Daemon won't connect to peers

Check that port 7120 (mainnet) or 17120 (testnet) is open:

```bash
sudo ufw allow 7120/tcp
```

Add seed nodes manually if DNS seeds haven't propagated:

```bash
./src/kerrigan-cli addnode "seed1.kerrigan.network" "add"
./src/kerrigan-cli addnode "seed2.kerrigan.network" "add"
```
