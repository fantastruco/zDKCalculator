"""Validate SVG independence, vector content, IDs, catalog entries and PNG size."""
from pathlib import Path
import json
import re
import struct
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1] / "asset" / "ui"


def check_svg(path):
    text = path.read_text(encoding="utf-8")
    tree = ET.fromstring(text)
    ids = [node.attrib["id"] for node in tree.iter() if "id" in node.attrib]
    assert len(ids) == len(set(ids)), f"Duplicate SVG IDs: {path}"
    for node in tree.iter():
        assert node.tag.rsplit("}", 1)[-1] not in {"image", "script", "foreignObject"}, f"Non-vector or active content: {path}"
        for key, value in node.attrib.items():
            if key.rsplit("}", 1)[-1] == "href":
                assert value.startswith("#"), f"External SVG resource: {path}"
    for reference in re.findall(r"url\(#([^)]+)\)", text):
        assert reference in ids, f"Unresolved SVG reference {reference}: {path}"
    assert "data:image" not in text, f"Raster data in SVG: {path}"
    return tree


def main():
    catalog = json.loads((ROOT / "catalog.json").read_text(encoding="utf-8"))
    assets = catalog["assets"]
    assert len(assets) == 38 and len({a["id"] for a in assets}) == 38
    for asset in assets:
        tree = check_svg(ROOT / asset["svg"])
        w, h = asset["width"], asset["height"]
        assert tree.attrib["viewBox"] == f"0 0 {w} {h}"
        png = (ROOT / asset["png_4x"]).read_bytes()
        assert png[:8] == b"\x89PNG\r\n\x1a\n"
        assert struct.unpack(">II", png[16:24]) == (w * 4, h * 4), f"Wrong PNG resolution: {asset['id']}"
        assert png[25] == 6, f"PNG must have RGBA transparency: {asset['id']}"
        if asset["stretch_insets"]:
            l, t, r, b = asset["stretch_insets"]
            assert l + r < w and t + b < h
    check_svg(ROOT / "contact-sheet.svg")
    assert len(list((ROOT / "svg").rglob("*.svg"))) == 38
    assert len(list((ROOT / "png" / "4x").rglob("*.png"))) == 38
    print("Validated 38 standalone vectors, 38 RGBA PNG exports at 4x and the vector contact sheet.")
    mobile = ROOT / "mobile"
    if (mobile / "catalog.json").exists():
        states = json.loads((mobile / "catalog.json").read_text())
        assert len(states) == 42 and len({s["resource"] for s in states}) == 42
        for state in states:
            check_svg(mobile / state["svg"])
            png = (mobile / state["png"]).read_bytes()
            assert struct.unpack(">II", png[16:24]) == (state["width"]*state["scale"],state["height"]*state["scale"])
            assert png[25] == 6
        assert len(list((mobile / "svg").rglob("*.svg"))) == 42
        assert len(list((mobile / "png").rglob("*.png"))) == 42
        print("Validated 42 embedded mobile skin states (normal, hover, active).")


if __name__ == "__main__":
    main()
