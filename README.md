# LossGauge

SKSE plugin for Skyrim SE/AE that makes a portion of health damage temporarily unrecoverable.

Inspired by the Loss Gauge system from **Dragon's Dogma**.

Loss is restored by sleeping and is stored per save using SKSE serialization.

## Requirements

- SKSE64
- Address Library for SKSE Plugins

## Configuration

`Data/SKSE/Plugins/LossGauge.toml`

```toml id="xotqhz"
[LossGauge]
LossRatio = 0.25

[Recovery]
SleepRecovery = true
FullRecoveryHours = 8.0

[Debug]
DebugLogging = false
```

`LossRatio` controls how much damage becomes Loss.

With `LossRatio = 0.25`, taking 20 damage adds 5 Loss.

`FullRecoveryHours` controls the cumulative sleep time required to recover the Loss present at the start of a recovery cycle.

With the default 8 hours:

| Sleep | Recovery |
|---:|---:|
| 1h | 12.5% |
| 2h | 25% |
| 4h | 50% |
| 6h | 75% |
| 8h | 100% |

Sleep sessions are cumulative. Sleeping 4 hours twice completes one 8-hour recovery cycle.

If sleep is interrupted, only the elapsed sleep time counts.

## Build

Requires CommonLibSSE NG and xmake.

```cmd id="wy3gpd"
xmake
```

## Status

Work in progress.

Implemented:

- Damage-based Loss
- Healing cap
- SKSE serialization
- Sleep recovery
- Cumulative sleep recovery
- Interrupted sleep handling
- TOML configuration