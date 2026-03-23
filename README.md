# logos-zone-sequencer-module

A Logos Core IComponent module that enables zone inscription on the Logos blockchain via [zone-sdk](https://github.com/logos-blockchain/logos-blockchain/tree/main/zone-sdk).

## Architecture

```
Logos module (Qt)
    ↓ invokeRemoteMethod (QtRO)
logos-zone-sequencer-module (C++ Qt plugin)
    ↓ C FFI
zone-sequencer-rs (Rust, embeds tokio + zone-sdk)
    ↓ HTTP
Logos blockchain node → devnet/mainnet
```

## Q_INVOKABLE Interface

| Method | Description |
|--------|-------------|
| `set_node_url(url)` | HTTP endpoint of the blockchain node (default: `http://localhost:8080`) |
| `set_signing_key(hex)` | Ed25519 signing key as 64-char hex (32 bytes). Channel ID derived from public key. |
| `set_checkpoint_path(path)` | File path to load/save sequencer checkpoint. Required for chaining inscriptions. Fresh channels start without a checkpoint. |
| `publish(data)` → `QString` | Inscribes `data` as a zone block. Returns local inscription ID on success, error string on failure. |

## Usage via logoscore

```bash
logoscore -m ~/.local/share/Logos/LogosAppNix/modules \
  -l liblogos_zone_sequencer_module \
  -c "liblogos_zone_sequencer_module.set_node_url(http://192.168.0.209:8080)" \
  -c "liblogos_zone_sequencer_module.set_signing_key(<64-char-hex-key>)" \
  -c "liblogos_zone_sequencer_module.set_checkpoint_path(/tmp/sequencer.checkpoint)" \
  -c "liblogos_zone_sequencer_module.publish(hello world)"
```

## Checkpoint

The zone-sdk requires a checkpoint for chain continuity — each inscription references the previous one. This module:

1. Loads checkpoint from `checkpoint_path` before publishing
2. Saves updated checkpoint after each successful inscription

**Fresh channel** (no prior inscriptions): omit or leave checkpoint file absent — the first inscription bootstraps automatically.

**Lost checkpoint**: re-bootstrap by running `logos-blockchain-node inscribe` once with the same key, then copy its checkpoint file.

## Channel ID derivation

Channel ID = Ed25519 public key of the signing key:

```bash
# Deterministic key from a name + secret
SIGNING_KEY=$(echo -n "board-name:secret" | sha256sum | cut -d" " -f1)
```

## Building

```bash
# Build Rust FFI lib first
git clone https://github.com/jimmy-claw/zone-sequencer-rs ../zone-sequencer-rs
cd ../zone-sequencer-rs && cargo build --release

# Build Qt plugin
cd logos-zone-sequencer-module
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<qt6-path>
make -j$(nproc)
```

## Installing

```bash
MODULES_DIR=~/.local/share/Logos/LogosAppNix/modules/zone_sequencer_module
mkdir -p $MODULES_DIR
cp build/liblogos_zone_sequencer_module.so $MODULES_DIR/
cp ../zone-sequencer-rs/target/release/libzone_sequencer_rs.so $MODULES_DIR/
cp manifest.json $MODULES_DIR/
# Also copy to root for logoscore -l discovery
cp build/liblogos_zone_sequencer_module.so $(dirname $MODULES_DIR)/
cp ../zone-sequencer-rs/target/release/libzone_sequencer_rs.so $(dirname $MODULES_DIR)/
```

## Confirmed working

- ✅ Tested on Logos devnet (March 2026)
- ✅ Used by [yolo-ng](https://github.com/jimmy-claw/yolo-ng) — decentralized text board

## Related

- [zone-sequencer-rs](https://github.com/jimmy-claw/zone-sequencer-rs) — Rust FFI library
- [zone-inscribe](https://github.com/jimmy-claw/zone-inscribe) — standalone CLI tool
- [yolo-ng](https://github.com/jimmy-claw/yolo-ng) — consumer module
