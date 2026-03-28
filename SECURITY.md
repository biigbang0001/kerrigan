# Security Policy

## Reporting a Vulnerability

If you discover a security vulnerability in Kerrigan, please report it responsibly.

**Do NOT open a public GitHub issue for security vulnerabilities.**

### Contact

Email: **admin@kerrigan.network**

### What to include

- Description of the vulnerability
- Steps to reproduce
- Affected version(s)
- Impact assessment (crash, fund loss, consensus split, etc.)

### Response timeline

- **Acknowledgment**: within 48 hours
- **Initial assessment**: within 1 week
- **Fix timeline**: depends on severity, typically 1-4 weeks

### Scope

- Kerrigan daemon (kerrigand, kerrigan-cli, kerrigan-qt)
- Consensus rules and validation logic
- P2P network protocol
- RPC interface
- Wallet and key management
- HMP (Hashrate Metering Protocol) subsystem

### Out of scope

- Pool software (kerrigan-network/miningcore, kerrigan-network/s-nomp)
- Third-party services, explorers, or websites
- Social engineering attacks

## Supported Versions

| Version | Supported |
|---------|-----------|
| 1.0.x   | Yes       |
