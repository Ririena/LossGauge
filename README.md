# LossGauge

SKSE plugin for Skyrim SE/AE that makes a portion of health damage temporarily unrecoverable.

Inspired by the Loss Gauge system from **Dragon's Dogma**.

## Requirements

- SKSE64
- Address Library for SKSE Plugins
- PrismaUI
- SKSE Menu Framework

## Features

- Configurable damage-to-Loss ratio
- Healing limited by Recoverable Health
- Loss recovery through sleeping
- Cumulative sleep recovery
- Optional natural health regeneration
- Per-save SKSE serialization
- PrismaUI Loss bar
- In-game UI editor

## How It Works

Loss is tracked separately without changing the player's Max Health.

```text
Recoverable Health = Max Health - Loss
```

With `LossRatio = 0.25`, taking 20 damage adds 5 Loss.

Healing and health regeneration cannot normally go above Recoverable Health until the Loss is restored.

## Sleep Recovery

Loss is recovered by sleeping.

`FullRecoveryHours` controls how much cumulative sleep is required for full recovery.

| Sleep | Recovery |
|---:|---:|
| 1h | 12.5% |
| 2h | 25% |
| 4h | 50% |
| 6h | 75% |
| 8h | 100% |

Sleep is cumulative. Waiting does not recover Loss.

## HUD

LossGauge uses PrismaUI to display a thin Loss bar above the normal health bar.

The bar grows from right to left as Loss increases and is hidden at 0% Loss.

The in-game UI editor can be used to change its position, size, color, opacity, and animation.

## Configuration

Configuration is stored in:

```text
Data/SKSE/Plugins/LossGauge.toml
```

```toml
[LossGauge]
LossRatio = 0.25

[Recovery]
SleepRecovery = true
FullRecoveryHours = 8.0

[Health]
NaturalHealthRegeneration = true

[Debug]
DebugLogging = false

[UI]
PositionX = 56.0
PositionY = 118.0
Width = 246.0
Height = 4.0

ColorR = 90
ColorG = 90
ColorB = 90

Opacity = 0.95
EnableAnimation = true
AnimationDuration = 0.12
```

Settings can also be changed in-game through SKSE Menu Framework.

## Build

Requires CommonLibSSE-NG, xmake, and toml++.

```cmd
xmake build -j 1
```

## Credits

Inspired by the Loss Gauge system from **Dragon's Dogma**.

Built with SKSE, CommonLibSSE-NG, and PrismaUI.