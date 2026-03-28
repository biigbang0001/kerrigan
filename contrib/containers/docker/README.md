# Kerrigan Docker Image

Multi-stage Docker build that compiles `kerrigand` and `kerrigan-cli` from source and
packages them into a minimal Ubuntu 24.04 runtime image.

## Quick Start

### Build

```bash
docker build -t kerrigan -f contrib/containers/docker/Dockerfile .
```

### Run (mainnet)

```bash
docker run -d \
  --name kerrigand \
  -p 7120:7120 \
  -p 7121:7121 \
  -v kerrigan-data:/home/kerrigan/.kerrigan \
  kerrigan
```

### Run (testnet)

```bash
docker run -d \
  --name kerrigand-testnet \
  -p 17120:17120 \
  -p 17121:17121 \
  -v kerrigan-testnet:/home/kerrigan/.kerrigan \
  kerrigan kerrigand -testnet
```

## Configuration

### Via kerrigan.conf

Mount a configuration file into the container:

```bash
docker run -d \
  --name kerrigand \
  -v /path/to/kerrigan.conf:/home/kerrigan/.kerrigan/kerrigan.conf:ro \
  -v kerrigan-data:/home/kerrigan/.kerrigan \
  -p 7120:7120 \
  -p 7121:7121 \
  kerrigan
```

### Via command-line arguments

Pass arguments after the image name:

```bash
docker run -d kerrigan kerrigand -testnet -rpcallowip=0.0.0.0/0 -rpcbind=0.0.0.0
```

## Persistent Data

Chain data, wallet files, and configuration are stored in `/home/kerrigan/.kerrigan`.
Always mount a named volume or host directory to persist data across container restarts.

## Ports

| Port  | Protocol | Network  |
|-------|----------|----------|
| 7120  | P2P      | Mainnet  |
| 7121  | RPC      | Mainnet  |
| 17120 | P2P      | Testnet  |
| 17121 | RPC      | Testnet  |

## Using kerrigan-cli

```bash
docker exec kerrigand kerrigan-cli getblockchaininfo
```

## Docker Compose

A `docker-compose.yml` is provided at the repository root:

```bash
docker compose up -d
```
