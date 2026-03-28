# Kerrigan version v23.1.0

This is a new minor version release, bringing new features, important bugfixes, and significant performance improvements.
This release is highly recommended for all nodes. All Masternodes are required to upgrade.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/kerrigan-network/kerrigan/issues>

# Upgrading and downgrading

## How to Upgrade

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over /Applications/Kerrigan-Qt (on Mac) or
kerrigand/kerrigan-qt (on Linux).

## Downgrade warning

### Downgrade to a version < v23.0.0

Downgrading to a version older than v23.0.0 is not supported, and will
require a reindex.

# Notable changes

## InstantSend performance improvements

This release includes a collection of changes that significantly improve InstantSend lock latency. When fully adopted
across the network, these changes are expected to reduce average InstantSend lock times by 25-50%, bringing average
lock confirmation times down to near 1 second. Key improvements include:

- **Quorum message prioritization**: Network message processing now separates quorum-related messages (DKG contributions,
  signing shares, recovered signatures, ChainLock signatures, InstantSend locks) into a dedicated priority queue. This
  ensures consensus-critical quorum operations are processed with lower latency than regular network traffic.
- **Proactive recovered signature relay**: Recovered signatures are now proactively pushed to connected nodes rather than
  waiting for them to be requested. InstantSend quorums are now fully connected for recovered signature relay, reducing
  the number of network hops required for IS lock propagation.
- **Multi-threaded signing shares processing**: The `CSigSharesManager` has been enhanced with a multi-threaded worker
  pool and dispatcher for parallel processing of signing shares, improving throughput during periods of high signing
  activity.
- **InstantSend height caching**: Block heights are now cached in a dedicated LRU cache, significantly reducing
  contention on `cs_main` during InstantSend lock verification.
- **Shared mutex conversions**: The `m_nodes_mutex` and `m_peer_mutex` have been converted from recursive mutexes to
  shared mutexes, allowing concurrent read access from multiple threads. This reduces lock contention for the most
  frequent network operations.
- **Reduced redundant work**: Peers that have requested recovered signature relay are no longer sent redundant `ISDLOCK`
  inventory announcements. Redundant signature validation during signing share processing has been eliminated.

## GUI refresh

The Masternode tab has been significantly overhauled with new status icons reflecting masternode ban state, type and
ban filters, context menus replacing address columns, elastic column widths, and a detailed masternode description
dialog. A dedicated masternode model now backs the list view.

The Governance and Masternode tabs can now be shown or hidden at runtime through the Options dialog without requiring a
client restart. The proposal model has been split out for better separation of concerns.

## Dust attack protection

A new GUI option allows users to enable automatic dust attack protection. When enabled, small incoming transactions from
external sources that appear to be dust attacks are automatically locked, excluding them from coin selection. Users can
configure the dust threshold in the Options dialog.

## Descriptor wallets no longer experimental

Descriptor-based wallets have been promoted from experimental status to a fully supported wallet type. Users can now
create and use descriptor wallets without the experimental warning.

## GUI settings migration

Configuration changes made in the Kerrigan GUI (such as the pruning setting, proxy settings, UPNP preferences) are now
saved to `<datadir>/settings.json` rather than to the Qt settings backend (Windows registry or Unix desktop config
files), so these settings will now apply to `kerrigand` as well, instead of being ignored.

Settings from `kerrigan.conf` are now displayed normally in the GUI settings dialog, instead of in a separate warning
message. These settings can now be edited because `settings.json` values take precedence over `kerrigan.conf` values.

## Other notable changes

* Masternodes now trickle transactions to non-masternode peers (as regular nodes do) rather than sending immediately,
  reducing information leakage while maintaining fast masternode-to-masternode propagation.
* The Send Coins dialog now warns users when sending to duplicate addresses in the same transaction.
* External signer (hardware wallet) support has been added as an experimental feature, allowing wallets to delegate
  signing to HWI-compatible external devices.
* Improved wallet encryption robustness and HD chain decryption error logging.
* Peers that re-propagate stale quorum final commitments (`QFCOMMIT`) are now banned starting at protocol version
  70239.
* Various race conditions in ChainLock processing have been fixed.

## P2P and network changes

- `PROTO_VERSION` has been bumped to `70240` with the introduction of protocol version-based negotiation of BIP324 v2
  transport short IDs for Kerrigan-specific message types. The `PLATFORMBAN` message has been added to the v2 P2P short ID
  mapping (short ID 168). When communicating with peers supporting version 70240+, this message uses 1-byte encoding
  instead of 13-byte encoding, reducing bandwidth. Compatible peers use compact encoding, while older v2 peers
  automatically fall back to long encoding.

## Updated RPCs

- `quorum dkginfo` now requires that nodes run in either watch-only mode (`-watchquorums`) or as an active masternode,
  as regular nodes do not have insight into network DKG activity.
- `quorum dkgstatus` no longer emits the return values `time`, `timeStr` and `session` on nodes that do not run in
  either watch-only or masternode mode.
- The `getbalances`, `gettransaction` and `getwalletinfo` RPCs now return a `lastprocessedblock` JSON object containing
  the wallet's last processed block hash and height at the time the result was generated.
- Fixed the BLS scheme selection in `protx revoke` and `protx update_service` to use the actual deployment state rather
  than hardcoded values.
- Fixed `protx register` to include the `Prepare` action for wallet unlock checks.
- Added a new `next_index` field in the response in `listdescriptors` to have the same format as `importdescriptors`

Changes to wallet related RPCs can be found in the Wallet section below.

## Updated settings

- The `shutdownnotify` option is used to specify a command to execute synchronously before Kerrigan has begun its
  shutdown sequence.

## Build System

- Kerrigan binaries now target Windows 10 and macOS 14 (Sonoma), replacing the previous targets of Windows 7 and
  macOS 11 (Big Sur).
- The minimum supported Clang version has been bumped to Clang 19 for improved C++20 support and diagnostics.

## Wallet

- CoinJoin denomination creation now respects the wallet's "avoid_reuse" setting. When the wallet has `avoid_reuse`
  enabled, change is sent to a fresh change address to avoid address/public key reuse. Otherwise, change goes back to
  the source address (legacy behavior).
- CoinJoin masternode tracking (`vecMasternodesUsed`) is now shared across all loaded wallets instead of per-wallet,
  improving mixing efficiency.

## GUI changes

- Masternode tab redesigned with new status icons, type/ban filters, context menus, elastic column widths, and
  detailed masternode description dialog.
- Runtime show/hide of Governance and Masternode tabs through Options without restart.
- Dust attack protection option added to Options dialog.
- Duplicate recipient warning in Send Coins dialog.
- Auto-validation of governance proposals as fields are filled in.
- Wallet rescan option now available when multiple wallets are loaded (rescan remains one wallet at a time).
- Improved CreateWalletDialog layout.
- Kerrigan-specific font infrastructure extracted to dedicated files with `FontInfo` and `FontRegistry` classes, supporting
  arbitrary fonts and dynamic font weight resolution.
- Fixed precision loss in proposal generation.
- Fixed crash when changing themes after mnemonic dialog was shown.

See detailed [set-of-changes][set-of-changes].

# Credits

Thanks to everyone who directly contributed to this release:

- Kittywhiskers Van Gogh
- Konstantin Akimov
- PastaPastaPasta
- thephez
- UdjinM6
- Vijay
- zxccxccxz

As well as everyone that submitted issues, reviewed pull requests and helped
debug the release candidates.

[set-of-changes]: https://github.com/kerrigan-network/kerrigan/compare/v23.0.2...v23.1.0
