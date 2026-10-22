# Dokkan Defense Calculator

A portable Windows defense calculator for Dragon Ball Z Dokkan Battle, written in C++17 with Dear ImGui and Direct3D 11.

[Download the latest release](https://github.com/fantastruco/zDKCalculator/releases/latest).

## Features

- Separate Unit, Boss and Info tabs in a resizable portrait interface.
- Unit DEF, Leader Skills, Link Skills, Hidden Potential Type DEF Boost, guard and damage reduction.
- Conditional effects for Entrance Animation, Basic Effect, When Attacking, When Receiving Attack, Super Attack, Ultra Super Attack, Ally Support and Active Skill.
- Independent enemy Normal ATK and Super Attack ATK, or base ATK with an SA multiplier.
- Damage ranges, remaining HP, hits until K.O. and calculation breakdowns.
- Unit presets, import/export and saved sessions.
- Live thousands separators, smooth vertical scrolling, scalable text and neon controls.
- Always on Top and verified updates through Settings.

Calculation works offline. A network connection is required only to check or download updates.

## Using the calculator

Run `DokkanDefenseCalculator.exe`. Requires Windows 10 or 11, 64-bit. Artwork and Windows icons are embedded; no separate asset folder is needed.

**Unit → Stats** accepts character stats and total Leader Skill DEF. Two 220% leaders are entered as **440**. **Manual DEF** uses a known DEF stat while retaining guard, DR and other protections.

**Unit → Effects → + Add** opens the full-screen effect picker. Select a category, enter the boost or skill and configure its activation conditions. Click an existing effect to edit it. Apply commits changes; Cancel or Escape discards the draft. Ordinary boosts add within their calculation bracket. `Multiplicative` applies an explicit independent factor. [Ally support and effect ordering](docs/zDKSupportEffects.md) explains the support brackets.

**Unit → Setup** contains team HP, Type DEF Boost and unit presets. **Hidden Potential 55%** selects level 6; **Hidden Potential 100%** selects level 11; **Custom** accepts a manual level. These are presets for the Type DEF Boost level, rather than an automatic character build.

**Boss → Stats** accepts separate full Normal and SA ATK, or base ATK plus an SA multiplier. **Effects** adds conditional enemy boosts. **Skills** contains enemy DEF, ATK Down sources, DEF ignored and manual critical-hit rules. Enter only effects that apply to the chosen enemy.

**Info → Damage / Tanking / Calcs** shows results for the selected attack, HP and protection details, and the calculation breakdown. **Copy calc** copies the encounter and results as plain text.

Turn and **Before attack / After attack** refer to the configured encounter and the unit's attack timing, including normal attacks. The calculator evaluates the supplied scenario; it does not automatically simulate a full attack sequence or track each buff's duration.

## Interface

Drag any window border or corner to scale the interface while retaining the 540:980 client-area aspect ratio. Text, controls, spacing and artwork scale together; fonts rebuild at the final size for sharp rendering. Monitor changes refresh DPI-aware Windows icons. Maximizing into a landscape window is disabled.

Use the top buttons to select Unit, Boss or Info and the lower bar to select a subview. Hold and drag vertically, use the wheel or drag the custom scrollbar to scroll. Each page retains its scroll state. Text fields and dropdowns preserve normal editing behavior.

The Settings gear contains **Always on Top**, **Animations & glow**, **App updates**, calculation reference, **New calculation** and **Resume last session**. The header pin indicates Always on Top. Type selectors use AGL blue, TEQ green, INT purple, STR red and PHY gold.

Startup and New calculation begin with zero stats and no sample effects. Turn and stack counts begin at 1; neutral coefficients retain their mathematical identity.

## Presets and sessions

In **Unit → Setup**, the menu beside the preset dropdown provides Save, Import and Export. Loading a unit preset preserves the boss, turn, attack timing and team HP. Portable preset files use the `.dunit` extension.

Data is stored under `%LOCALAPPDATA%\DokkanDefenseLab\`:

| Data | Location |
|---|---|
| Unit presets | `presets\` |
| Last edited session | `session-v5.dokkan` |
| Window settings | `window-settings.dokkan` |
| Staged updates | `updates\` |

The data-folder name remains stable for compatibility. Startup opens an empty calculation; Resume last session restores saved work. Closing an untouched empty calculation preserves the previous saved session. Current formats are `DOKKAN_UNIT 4` and `DOKKAN_SESSION 5`; older supported files migrate when loaded, and legacy session files remain available.

## Calculation model

Every stat-building phase returns an integer before the next phase. Additive boosts combine within their phase; independent factors truncate separately. Guard applies after DEF, while its type factor applies before DEF. Type DEF Boost requires natural type advantage.

DEF phase truncation is supported by community examples. The calculator also enforces integer steps throughout incoming damage; available references do not establish every internal incoming-damage rounding boundary in the game. Results above 9,007,199,254,740,991 are rejected.

Zero is the mathematical damage floor. Small random in-game minimum damage is not modeled. The central damage value uses variance 1.015. Enemy critical hits and DEF Down have source-specific limits; dodge, nullification, absorption and revival are outside this model.

See [defense rules and evidence](docs/zDKDefenseResearch.md), [integer arithmetic](docs/zDKRoundingResearch.md) and [ally support](docs/zDKSupportEffects.md).

## Build and test

Requires Visual Studio 2022 or Build Tools with **Desktop development with C++** and the Windows SDK.

```powershell
.\build.ps1
.\build.ps1 -Run
```

Dependencies are vendored. Normal builds require neither Python nor Node.js. The MSVC runtime is statically linked. The build runs 257 engine, formatting and update-metadata checks, 114 interface checks, and isolated Windows updater tests covering SHA-256, tamper rejection, executable replacement and restart.

CMake is also supported:

```powershell
cmake -S . -B out -G "Visual Studio 17 2022" -A x64
cmake --build out --config Release
ctest --test-dir out -C Release --output-on-failure
```

GitHub Actions verifies public-source pushes and pull requests. See [release publishing and app updates](docs/zDKUpdates.md) for the publication command and update behavior.

## Source and assets

Project-owned C++ declarations use the `zDK` prefix, followed by their responsibility: `Calc`, `Effect`, `Store`, `UI`, `Win`, `Update` or `Test`. See [coding standards](docs/zDKCodingStandards.md). Shared formatting and naming checks are configured in `.clang-format` and `.clang-tidy`.

| Location | Purpose |
|---|---|
| `src/zDKCalculator.h`, `src/zDKIntegerMath.h` | Defense, damage and integer arithmetic |
| `src/zDKEffects.h` | Effect resolution, conditions and persistence |
| `src/zDKUI.h`, `src/zDKUISkin.h` | Editors, results, controls and animations |
| `src/zDKUINumberFormat.h` | Grouped numeric editing |
| `src/zDKWindows.cpp` | Win32, DPI, Direct3D and diagnostics |
| `src/zDKUpdater.h`, `src/zDKUpdateMetadata.h` | Update validation, download and installation |
| `asset/ui/` | Reconstructed SVG library and PNG exports |
| `asset/ui/mobile/` | Normal, hover and active skin states |
| `asset/icons/` | Icon master and individual sizes up to 1024 px |
| `tests/` | Engine, interface and updater checks |

[Asset editing instructions](asset/ui/README.md), [mobile skin generation](asset/ui/mobile/README.md) and [icon exports](asset/icons/README.md) describe regeneration. The Windows ICO contains 27 sizes from 16 to 256 px. Icon tests cover DPI scales from 100% to 800%.

Dear ImGui **v1.91.9b** and nlohmann/json **v3.12.0** are included under their MIT licenses in `third_party/`. Windows supplies Segoe UI fonts, WinHTTP HTTPS and BCrypt SHA-256.
