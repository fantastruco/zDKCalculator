# Ally support and effect organization

## Support calculation

Ordinary DEF support from an ally's passive adds to the unit's start-of-turn passive. [Razzer's guide](https://kandymanis.github.io/dokkanalytics/razzers-guide) demonstrates +260% own DEF and +40% INT LR Broly support as a ×4 bracket. The [2021 community guide](https://www.reddit.com/r/DBZDokkanBattle/comments/q3mz26/an_overly_long_guide_to_calculating_dokkan/) also places ally boosts in their corresponding phase and distinguishes Active Skill support.

In **Unit → Effects → + Add → Ally Support**, enter an optional ally name, the percentage and **Support source**:

| Source | Calculation bracket |
|---|---|
| Start-of-turn passive | Adds to SoT with the unit's passive and other allies |
| Mid-battle passive | Adds to Phase 2 for support already active in the scenario |
| Super Attack effect | Adds to SA raises, using the active raise count |
| Active Skill | Adds to Active Skill boosts |

Base DEF 1,000, no extra Leader Skill, own +200% and support +40% give **3,400 DEF**. Adding +100% when attacking gives **6,800 DEF**. A further 50% SA raise gives **10,200 DEF**. Each phase truncates before the next.

`Multiplicative` defaults to off. Enabling it deliberately applies a separate factor. Choose the Active Skill phase for Active Skill support rather than using this override.

Turn, HP, timing and enable conditions apply. Enter only support affecting the selected unit. Categories, rotation slots, ally attack order and buff duration are not detected automatically. Active raises include previous stacks still in effect; effects requiring the unit's attack can be limited to after attacking.

## Display order

1. Entrance Animation
2. Basic Effect
3. When Attacking
4. When Receiving Attack
5. Super Attack / Ultra Super Attack

SA and Ultra SA have separate headings and counts within the final group. Their DEF raises add in the same SA bracket. Active Skills have an additional section. Support appears under the section matching its source. Empty sections are hidden.

The view sorts indices without reordering stored effects. This preserves each row's identity and the arithmetic order of independent factors, whose truncation remainders can differ.

## Guard

Choose **Skill effect → Guards all attacks** inside any Unit effect category. Guard retains its activation conditions when changing categories. It has no percentage or independent multiplier. For SA/Ultra SA guard, configure Attack timing to match the scenario; complete attack sequences are not simulated.

Legacy Guard-category entries migrate into the same model while preserving their conditions.

## Persistence and verification

New files use `DOKKAN_UNIT 4` and `DOKKAN_SESSION 5`. Presets v2/v3 and sessions v2/v3/v4 remain readable. Resume checks `session-v5.dokkan` before older session files, which are retained.

`tests/zDKSupportTests.h` covers additive support, phases, conditions, SA/Ultra SA, stable order and migration. Interface checks add and edit support, change its source, combine Ultra SA raises and select guard in effect editors.
