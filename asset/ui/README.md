# UI Interface — vector asset library

38 separate UI components reconstructed from `../UI_Interface.png`.

## Files

- **`svg/`**: individual, editable SVG masters, grouped by component type.
- **`png/4x/`**: transparent RGBA PNG exports at four times each SVG's native dimensions.
- **`contact-sheet.png`**: numbered visual catalog of every component.
- **`contact-sheet.svg`**: the same catalog with embedded vector geometry, suitable for zooming.
- **`catalog.json`**: names, paths, native dimensions, original reference bounds and stretch insets.

The original source sheet is preserved. No external fonts, linked images, base64 bitmaps or source-sheet crops are embedded in the SVG assets. Each SVG opens independently.

## Reconstruction and quality

These are geometric reconstructions, rather than enlarged raster crops or a literal automatic trace. The source silhouettes, neon palette, diagonal accents and corner motifs are retained. The original distressed/grain detail has been rebuilt as clean vector hatch patterns and paths, so fine texture is intentionally regularized rather than copying individual noisy pixels.

Exterior areas are transparent. Panel and button interiors have a dark fill; symbols, dividers and decorative textures retain transparency between their shapes. The preview catalog uses a checkerboard to make transparency visible.

The SVGs scale to any resolution. The 4× PNGs are freshly rendered from the vectors, not upscaled from the reference sheet. Examples:

| Asset | Native size | PNG export |
|---|---:|---:|
| `header-wide` | 1286 × 133 | 5144 × 532 |
| `panel-main` | 754 × 483 | 3016 × 1932 |
| `slot-crosshatch` | 138 × 139 | 552 × 556 |

The coordinate bounds of some neighboring decorations overlap in the reference sheet. They are reconstructed as separate graphics, so their output files do not include pieces of adjacent assets.

## Categories

| Folder | Contents | Count |
|---|---|---:|
| `frames` | Wide header, section banner and striped footer | 3 |
| `panels` | Main, tabbed and compact panels | 4 |
| `slots` | Six individual square frames | 6 |
| `buttons` | Ornate, capped, filled, compact and slim controls | 9 |
| `dividers` | Technical, star and notched dividers | 4 |
| `symbols` | Three orbs and two four-point sparkles | 5 |
| `tags` | Small and slanted tags | 2 |
| `indicators` | Chevron sequence and arrow progress track | 2 |
| `textures` | Dissolve, halftone and diagonal scratch overlays | 3 |

Use the catalog for the authoritative list of names and categories.

## Editing

The palette uses `#ff087a` for the main neon and `#ff539f` for highlights. Change those values in an SVG, or change `PINK` and `LIGHT` in the generator to rebuild the library.

Frame SVGs contain named layers for the outline, accents, texture and glow where applicable. Glow is a separate SVG filter layer, so its opacity can be adjusted or the layer removed without changing the hard outline. Patterns, gradients and clipping paths are local to each file.

For frames and buttons, preserve the original aspect ratio or use the suggested **nine-slice** insets in `catalog.json`. Insets are `[left, top, right, bottom]` in native SVG units; multiply them by four when cutting the PNG exports. Symbols, textures and decorative dividers should scale uniformly. Insets are starting guides for layout and can be adjusted for a particular control.

For the existing Direct3D 11/WIC application renderer, PNG exports can be loaded as textures. Their logical UI size is the native SVG size, multiplied by Windows DPI scaling; the file itself provides four pixels per logical unit. The SVG masters remain the editable source for future exports.

## Rebuild

SVG generation uses only Python's standard library. PNG rendering uses Node.js with `sharp` available in its module search path.

```powershell
python scripts/build_ui_assets.py
node scripts/render_ui_assets.cjs
python scripts/validate_ui_assets.py
```

Run these commands from the project root. The validation checks SVG structure, unique IDs, local references, absence of raster content, transparent PNG format and exact 4× dimensions.
