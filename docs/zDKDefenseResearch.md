# Defense calculation model

This document describes the calculator's arithmetic, community references and known limits. Community guides provide reproducible examples; they are not an official specification of the game.

## References

| Source | Coverage | Limits |
|---|---|---|
| [LR_AlternativeForce: How Damage Taken Works](https://www.reddit.com/r/DBZDokkanBattle/comments/weidle/how_damage_taken_works_dokkan_battle_complete/) | Original damage guide with linked examples | Predates Domains and recent enemy critical hits; linked documents require separate verification |
| [Razzer's Calcing Guide](https://kandymanis.github.io/dokkanalytics/razzers-guide) | DEF phases, truncation and enemy Broly calculations | Critical-hit explanations and examples are not fully consistent |
| [Helioseckta: the gap between 55% and 100%](https://www.reddit.com/r/DBZDokkanBattle/comments/1x9jbvh/the_gap_between_55_and_100_why_this_will_continue/) | SSJ4 Gogeta receiving-attack DEF examples | Examples apply to the stated passives and conditions |
| [PrismAzure: How Defense Works](https://www.reddit.com/r/DBZDokkanBattle/comments/d3cakb/how_defense_works_in_dokkan_battle_complete_guide/) | Support, SA stacks and items | Older guard and Link Skill explanations need qualification |
| [General and normal-only DR example](https://www.reddit.com/r/DBZDokkanBattle/comments/xmxsvv/) | Demonstrates separate general and attack-specific DR factors | Does not establish that every DR line is an independent factor |
| [Domain damage discussion](https://www.reddit.com/r/DBZDokkanBattle/comments/1bb6ekm/) | Distinguishes increased enemy ATK from increased final damage received | Exact Domain wording and conditions matter |
| [DokkanStats: Damage Received](https://dokkanstats.com/en/guides/how-to-compute-damage-received-v68kfr/) | ATK Down sources and received damage | Independent calculator documentation |
| [Dokkan Entropy function tables](https://wiki.dokkanentropy.com/functions/) | Attack, receiving and appearance conditions | Work in progress with unknown parameters |
| [Enemy critical-hit discussion, 2024](https://www.reddit.com/r/DBZDokkanBattle/comments/1fhe13n/) and [2026](https://www.reddit.com/r/DBZDokkanBattle/comments/1x7ntt3/the_character_whose_whole_thing_is_countering/) | Conflicting guard and critical-hit interpretations | A universal rule requires controlled, reproducible evidence |

See [integer arithmetic](zDKRoundingResearch.md) and [ally support](zDKSupportEffects.md) for their specific rules.

## DEF when receiving an attack

Receiving boosts can activate before the unit attacks. The resolver evaluates normal and Super Attack DEF independently. Active boosts in the same Phase 2 bracket add together; that bracket multiplies the preceding phases. `Multiplicative` explicitly applies a separate factor.

| DEF before the receiving bracket | Normal with +200% | SA with +200% and +200% |
|---:|---:|---:|
| 446,528 | 1,339,584 | 2,232,640 |
| 554,393 | 1,663,179 | 2,771,965 |

The SA bracket is `1 + 2 + 2 = 5`. **Info → Damage** shows the two resulting DEF stats. The selected attack determines the detailed breakdown.

## Enemy ATK and Super Attack effects

Boss effects use **Phase 1**, **Phase 2**, **Domain / Territory** and **Item**. Ordinary boosts add within a phase; different phases multiply.

**Direct ATK** accepts independent normal and full SA values. **Base ATK + SA multiplier** derives the SA from base ATK and supports enemy SA raises and SA-origin ATK Down. Direct inputs must already include those SA effects; validation prevents applying them ambiguously.

Razzer's Broly example uses base ATK 1,200,000 and a +30% starting boost: normals reach 1,560,000. A 300% SA multiplier with a 50% raise gives 5,460,000 SA ATK; subsequent normals reach 2,340,000. With +150% initially and +100% in Phase 2, the corresponding values are 21,000,000 and 9,000,000.

Enter the active totals for the chosen scenario. Enemy raises, their caps and individual durations are not simulated automatically.

## ATK Down sources

**Boss → Skills** separates SA, passive and item ATK Down. In the modeled calculation, SA-origin ATK Down subtracts percentage points from the enemy multiplier; passive and item reductions apply their own factors.

With base ATK 1,000,000 and a 300% SA multiplier, 20% SA-origin ATK Down gives 2,800,000. Applying 30% passive and 20% item reductions then gives `2,800,000 × 0.7 × 0.8 = 1,568,000`.

Enemy immunity is configured manually by entering only reductions that apply. Imported configurations follow the same validation rules.

## Domains

A Unit effect named **Damage received increase (Domain)** multiplies damage after DEF and guard. A Domain increasing enemy ATK belongs in Boss Effects instead.

For neutral type, ATK 1,000,000 and DEF 400,000, a final-damage increase of 30% gives `(1,000,000 - 400,000) × 1.3 = 780,000`. With passive guard, the modeled result is `(1,000,000 × 0.8 - 400,000) × 0.5 × 1.3 = 260,000`.

Configure only effects whose class and activation conditions apply to the unit.

## Model limits

- Guard has a type factor before DEF and a guard factor after DEF. Type DEF Boost requires natural type advantage.
- General DR sources add by default. Attack-specific DR uses a separate factor. Independent factors require the explicit `Multiplicative` setting.
- SA stacks and SA support add in the SA bracket.
- Enemy critical hits are a manual scenario with editable type, guard and DEF-ignore behavior. Available community examples do not resolve every interaction with class, Type DEF Boost, DR and passive guard.
- DEF Down currently reduces final DEF. Its field does not claim to reproduce every SA debuff, stack interaction or source-specific rule.
- Conditions use the manually supplied turn and HP. The model does not capture start-of-turn HP, count turns since first appearance or track each stack's duration.
- Zero is the mathematical damage floor. Small random minimum damage is not modeled. The central result uses variance 1.015; it is not a guaranteed in-game roll.
- Dodge, nullification, absorption, revival, survival at K.O. and indirect AoE rules require separate models.

## Persistence and verification

Current formats are `DOKKAN_UNIT 4` and `DOKKAN_SESSION 5`. Presets v2/v3 and sessions v2/v3/v4 remain readable. Resume searches the current session first and then legacy files. Legacy files are retained, and missing fields use neutral defaults.

`tests/zDKResearchTests.h` covers community examples and compatibility. `build.ps1` runs engine and interface checks.
