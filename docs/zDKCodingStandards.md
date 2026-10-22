# zDK source conventions

Every project-owned C++ namespace, type, enumerator, function, method, field,
constant, parameter and local variable starts with `zDK`. The responsibility
follows the prefix, then a descriptive English name in PascalCase.

| Responsibility | Prefix | Example |
|---|---|---|
| Defense and damage arithmetic | `zDKCalc` | `zDKCalcCalculateBattle` |
| Skill effects, activation and profiles | `zDKEffect` | `zDKEffectResolveUnit` |
| Preset and session serialization | `zDKStore` | `zDKStoreSaveSession` |
| Interface, animation and skin rendering | `zDKUI` | `zDKUIDrawEffectModal` |
| Windows, Direct3D and application lifecycle | `zDKWin` | `zDKWinCreateRenderTarget` |
| Verification and interaction harnesses | `zDKTest` | `zDKTestExpectNear` |

The engine namespace is `zDK`; the application namespace is `zDKUI`.
An identifier keeps its owner's prefix when another module uses it. For example,
the UI reads `zDKCalcMinimumDamage` from a `zDKCalcDamageRange`.

Use names that explain the value or action. Write `zDKCalcDamageReductionPercent`
instead of `dr`, `zDKWinSwapChainDescription` instead of `sd`, and
`zDKUISelectedAttackKind` instead of `chosenAttack`. Include units such as
`Percent`, `Seconds`, `Pixels` or `Level` when they remove ambiguity. Spell out
local names; do not create names such as `zDKUIA` or `zDKWinTmp1`.

Source and test filenames also start with `zDK`. Generated C++ declarations must
follow the same convention; update `scripts/build_mobile_skin.py` when changing
the skin resource schema, then regenerate its output.

## External contracts

Do not rename Windows, Direct3D, Dear ImGui, C++ standard-library or imported
dependency declarations and members. `wWinMain` and the test executable's `main`
are runtime-required entry points; each is a small adapter to a `zDK` function.
`ImGui_ImplWin32_WndProcHandler` is an external backend declaration. Windows
configuration macros such as `NOMINMAX`, `UNICODE` and `_WIN32_WINNT` keep their
required names. The `ComPtr` import retains Microsoft's type name.

Serialized strings, file-format versions, widget IDs, diagnostic command-line
options, preset extensions and existing user-data paths are compatibility
contracts. Source renaming must not rewrite them. The application and executable
remain **Dokkan Defense Calculator** and `DokkanDefenseCalculator.exe`.

## Formatting and verification

`.clang-format` defines four-space indentation, Allman braces and a 110-column
target for project C++ files. `.clang-tidy` enables identifier-prefix checks for
project declarations with the runtime/backend entry-point exceptions above.
Do not apply project formatting or naming rules to `third_party/`.

From the workspace root, format project C++ sources with:

```powershell
$zDKSourceFiles = Get-ChildItem src,tests -File |
    Where-Object { $_.Extension -in '.h','.cpp' } |
    ForEach-Object { $_.FullName }
clang-format -i @zDKSourceFiles
```

Run `./build.ps1` after a refactor. It compiles with MSVC `/W4`, executes the
calculator tests and runs the hidden UI interaction checks. Verify changes to
serialization against existing presets and session versions. The ordinary C++
build needs neither Python nor Node; those tools belong to the optional asset
generation pipeline.
