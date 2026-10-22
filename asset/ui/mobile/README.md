# Mobile calculator skin

The application uses 14 components from the reconstructed vector library, each with three independent states: **normal**, **hover**, and **active**. The active state is also used by selected tabs and focused controls.

- `svg/normal`, `svg/hover`, `svg/active`: 42 editable vector masters.
- `png/normal`, `png/hover`, `png/active`: runtime texture exports.
- `catalog.json`: source dimensions, render density, state and resource IDs.

Panel fills are translucent, with a darker body under form content. Glow opacity, blur and edge color vary by state. All SVG files are standalone and contain vector geometry rather than embedded raster data. Runtime PNGs are rendered from those masters at 2× or 3× native resolution, according to the component's display size.

The C++ renderer uses nine-slice scaling for frame corners and layered native ImGui widgets for input handling. Only glow opacity pulses when animation is enabled. Text is rendered independently at the monitor's current DPI. Switching **Unit / Boss / Info** retains the encounter and each page's scroll state.

The generated resource files are `src/zDKUIAssets.h` and `src/zDKUIAssets.rcinc`. `src/zDKResources.rc` embeds every state alongside `asset/BackgroundMobile.png` and the original application icon. Generated C++ declarations use the `zDKUI` prefix.

From the project root:

```powershell
python scripts/build_mobile_skin.py
node scripts/render_mobile_skin.cjs
.\build.ps1
```

SVG generation requires Python's standard library. The render script requires Node.js and `sharp`; it skips unchanged exports. Normal C++ builds use the supplied PNG files and require neither tool.
