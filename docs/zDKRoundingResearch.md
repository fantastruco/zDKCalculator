# Integer arithmetic and rounding

## Evidence and scope

DEF construction truncates downward after each phase. Additive boosts within one phase combine before the phase is applied. [Razzer's guide](https://kandymanis.github.io/dokkanalytics/razzers-guide) provides phase ordering and examples reproduced by the test suite.

[Digimon-Flow's screenshot research](https://www.reddit.com/r/DBZDokkanBattle/comments/e4hu3m/how_dokkan_rounds_decimals_with_screenshots_and/) documents downward truncation in Leader Skills, starting passives and links. It also reports ATK exceptions involving Ki and SA multipliers. The [2021 community calculation guide](https://www.reddit.com/r/DBZDokkanBattle/comments/q3mz26/an_overly_long_guide_to_calculating_dokkan/) discusses these separately. Character Ki ATK is outside this defense calculator's scope.

For damage received, [DokkanStats](https://dokkanstats.com/en/guides/how-to-compute-damage-received-v68kfr/) describes truncation at the end of its formula. That does not establish every internal rounding boundary in the game. The calculator consistently uses integer steps throughout incoming damage; those intermediate boundaries remain a model policy pending controlled in-game verification.

## Arithmetic policy

Every stat stage accepts and returns an integer. The engine computes `floor(stat × numerator / denominator)` using integer multiplication and division, discarding the remainder before the next stage. For nonnegative values, truncation equals rounding down.

```text
137 with +17%: 137 × 117 / 100 = 160
160 with +13%: 160 × 113 / 100 = 180
```

Carrying the intermediate 160.29 instead would produce 181.

| Calculation | Order |
|---|---|
| Unit DEF | Base → Leader Skill → SoT → Domain → Item → Links → Active Skill → Phase 2 → SA effects |
| Boss ATK | SoT → Domain → Item → Phase 2 → manual adjustment; shared ATK supplies derived SA mode |
| ATK Down | Source-specific SA adjustment, passive reduction and item reduction |
| Effective DEF | DEF Down → general DEF ignored → critical DEF ignored |
| Incoming damage | ATK → unresolved manual adjustment → critical → general DR → attack-specific DR → type → variance → subtract DEF, with floor zero → guard → final damage increases |

Ordinary boosts add within their bracket. Each independent multiplier is applied and truncated separately. Normal and Super Attack paths resolve their own boosts and DEF.

Percentages and multipliers are coefficients, rather than stats. Guard's 0.8 coefficient is preserved. Coefficients normalize to ratios with up to nine decimal places of multiplier precision; stat products use integer arithmetic. Results above **9,007,199,254,740,991**, the maximum safe integer in the public and persisted representation, produce an explicit error.

HP thresholds retain their ratio so a condition such as 50.5% is not changed. Hits until K.O. uses upward division to count complete hits; that derived count is not a stat-building stage. HP percentages and health-bar fill are presentation ratios.

## Persistence and verification

Legacy rounding flags remain readable but normalize to enabled. They cannot disable integer steps. Current formats are `DOKKAN_UNIT 5` and `DOKKAN_SESSION 6`.

`tests/zDKRoundingTests.h` covers nonzero remainders, separate factors, direct and derived SA ATK, independent DR, final Domain damage, DEF ignored, large ratios, overflow rejection and legacy presets. Community DEF examples and interface tests remain part of the build.
