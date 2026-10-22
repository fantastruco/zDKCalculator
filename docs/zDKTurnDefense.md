# Turn-by-turn attack sequence

The attack plan is an ordered, deterministic scenario containing Normal Attack, Super Attack and Ultra Super Attack entries. It records the unit's attacks within one turn, rather than rolling additional-attack probabilities. There can be up to 16 entries; this is an editor limit, not a claimed game maximum.

## Starting point and raises

SA and Ultra SA effect counts describe raises already active before the first planned attack. New SA/Ultra entries start at zero raises. Keep only previous raises that remain active this turn; durations and expiration are configured manually.

Each planned SA adds one activation to the unit's Super Attack effects. An Ultra SA adds one activation to its Ultra Super Attack effects. Both contribute to the same SA-effect bracket. A normal additional adds no SA raise. Ally Support from an ally's SA remains active without repeating whenever this unit attacks.

Choose the actual attack type for each additional. Selecting an Ultra SA explicitly models that scenario; the calculator does not infer whether a particular character can launch one.

## Passive build-up

A Phase 2 DEF effect can grow with each attack, each Super Attack (including Ultra SA), or each Ultra SA. Enter DEF per activation, activations already accumulated, and a maximum activation count; zero maximum means uncapped. Ordinary amounts add within Phase 2 before that phase is applied. `Multiplicative` retains its explicit independent-factor behavior.

Previously accumulated build-up contributes before the first attack. A normal can add passive build-up without adding an SA raise. Enabled state, turn, HP and attack-timing conditions still apply.

## Recalculation

Each point is rebuilt from base stats through the existing integer stages. It never multiplies the previous displayed DEF by another SA factor. The timeline preserves separate DEF and maximum incoming damage against normals and Super Attacks, including receiving-specific effects and DEF ignored. Inputs remain unchanged.

Manual DEF remains fixed; base DEF is needed to project DEF-building effects. Boss stats and HP remain fixed throughout the preview. The plan does not simulate enemy attacks between the unit's attacks, change HP or expire earlier stacks automatically.

[Razzer's guide](https://kandymanis.github.io/dokkanalytics/razzers-guide) gives the regression example: Wrathful Broly reaches 431,791 DEF before attacking; three attacks with two SA give a Phase 2 multiplier of 2.5 and an SA-effect multiplier of 1.4, producing 1,511,267 DEF after phase truncation. The engine tests reproduce this example and check mixed SA/Ultra/normal order, retained raises, support, build-up caps, conditional effects and manual DEF.

## Presets and sessions

The attack plan and passive build-up settings belong to the unit preset. New files use `DOKKAN_UNIT 5` and `DOKKAN_SESSION 6`. Presets v2–v4 and sessions v2–v5 remain readable. Old profiles retain their saved active raises and start with an empty attack plan. New sessions use `session-v6.dokkan`; Resume also searches previous session files without deleting them. Invalid or truncated plans fail to load without replacing the current data.
