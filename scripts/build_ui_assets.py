"""Reconstruct the UI_Interface reference as standalone, editable vector assets.

No tracing/upscaling dependency: outlines, hatch work and highlights are geometry.
The original raster remains the visual reference and is never embedded in an SVG.
"""
from pathlib import Path
from html import escape
import json
import math
import random
import struct

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "asset" / "ui"
PINK = "#ff087a"
LIGHT = "#ff539f"

# Bounds refer to the supplied 1774 x 887 reference, not an atlas used at runtime.
# Decorative elements whose bounding rectangles overlap are reconstructed separately.
ASSETS = [
    ("header-wide", "Wide header", "frames", (14, 14, 1300, 147), "header", (74, 24, 74, 24)),
    ("divider-tech", "Technical divider", "dividers", (1309, 22, 1762, 71), "rail", None),
    ("divider-star", "Star divider", "dividers", (1309, 50, 1762, 153), "star-rail", None),
    ("panel-main", "Main panel", "panels", (12, 153, 766, 636), "main", (74, 54, 74, 54)),
    ("panel-tabbed", "Tabbed panel", "panels", (776, 149, 1280, 395), "tabbed", (115, 48, 55, 48)),
    ("slot-crosshatch", "Crosshatch slot", "slots", (1297, 155, 1435, 294), "slot-0", (25, 25, 25, 25)),
    ("slot-distressed", "Fine-edge slot", "slots", (1456, 155, 1596, 294), "slot-1", (20, 20, 20, 20)),
    ("slot-heavy", "Heavy-corner slot", "slots", (1614, 155, 1756, 295), "slot-2", (30, 30, 30, 30)),
    ("slot-square", "Square slot", "slots", (1297, 296, 1435, 433), "slot-3", (15, 15, 15, 15)),
    ("slot-diagonal", "Diagonal-corner slot", "slots", (1456, 296, 1596, 435), "slot-4", (24, 24, 48, 48)),
    ("slot-notched", "Notched slot", "slots", (1614, 296, 1756, 435), "slot-5", (24, 24, 24, 24)),
    ("button-ornate", "Ornate button", "buttons", (777, 402, 1275, 488), "ornate", (56, 24, 56, 24)),
    ("button-capped", "Capped button", "buttons", (779, 496, 1275, 585), "capped", (58, 24, 42, 24)),
    ("banner-section", "Section banner", "frames", (11, 640, 766, 757), "section", (64, 28, 64, 28)),
    ("strip-left", "Left detail strip", "buttons", (777, 596, 1022, 640), "strip-left", (24, 12, 38, 12)),
    ("strip-right", "Right detail strip", "buttons", (1034, 596, 1270, 640), "strip-right", (26, 12, 26, 12)),
    ("footer-striped", "Striped footer", "frames", (13, 775, 767, 867), "footer", (132, 24, 50, 24)),
    ("divider-notch", "Notched divider", "dividers", (398, 763, 764, 785), "notch-line", None),
    ("button-filled", "Filled button", "buttons", (777, 650, 1051, 756), "filled", (36, 28, 36, 28)),
    ("button-compact", "Compact button", "buttons", (1063, 665, 1267, 723), "compact", (28, 16, 28, 16)),
    ("button-chevron", "Chevron button", "buttons", (777, 761, 1045, 863), "chevron", (45, 28, 42, 28)),
    ("panel-compact-glow", "Compact glow panel", "panels", (1053, 749, 1267, 863), "compact-panel", (30, 28, 30, 28)),
    ("orb-reticle", "Reticle orb", "symbols", (1293, 449, 1382, 538), "orb-reticle", None),
    ("orb-segmented", "Segmented orb", "symbols", (1394, 450, 1481, 538), "orb-segmented", None),
    ("orb-scratch", "Scratch orb", "symbols", (1488, 450, 1576, 538), "orb-scratch", None),
    ("sparkle-diamond", "Diamond sparkle", "symbols", (1580, 447, 1664, 539), "sparkle", None),
    ("sparkle-cross", "Cross sparkle", "symbols", (1663, 439, 1764, 542), "cross", None),
    ("tag-small", "Small tag", "tags", (1283, 548, 1398, 604), "tag", (24, 16, 24, 16)),
    ("tag-slanted", "Slanted tag", "tags", (1398, 551, 1585, 607), "slanted", (36, 14, 36, 14)),
    ("divider-double-star", "Double-star divider", "dividers", (1582, 540, 1750, 609), "double-star", None),
    ("button-corner", "Corner button", "buttons", (1283, 604, 1492, 667), "corner", (46, 18, 26, 18)),
    ("indicator-chevron", "Chevron indicator", "indicators", (1504, 621, 1756, 670), "indicator", None),
    ("progress-arrow", "Arrow progress track", "indicators", (1284, 667, 1492, 705), "progress", (20, 10, 36, 10)),
    ("button-slim", "Slim button", "buttons", (1284, 705, 1494, 759), "slim", (25, 16, 25, 16)),
    ("panel-compact", "Compact panel", "panels", (1283, 755, 1495, 866), "lower-panel", (45, 26, 26, 35)),
    ("texture-dissolve", "Dissolve highlight", "textures", (1496, 673, 1755, 733), "dissolve", None),
    ("texture-halftone", "Halftone fade", "textures", (1496, 734, 1755, 793), "halftone", None),
    ("texture-scratch", "Diagonal scratch", "textures", (1498, 794, 1757, 867), "scratch", None),
]


def n(value):
    return f"{value:.3f}".rstrip("0").rstrip(".")


def path(points, close=True):
    return "M" + " L".join(f"{n(x)},{n(y)}" for x, y in points) + (" Z" if close else "")


def poly(points, fill=PINK, opacity=1):
    return f'<path d="{path(points)}" fill="{fill}" opacity="{n(opacity)}"/>'


def line(points, width=1.2, color=PINK, opacity=1):
    return f'<path d="{path(points, False)}" fill="none" stroke="{color}" stroke-width="{n(width)}" opacity="{n(opacity)}" stroke-linecap="square" stroke-linejoin="miter"/>'


def shape(d, fill="none", stroke=PINK, width=2.5, opacity=1):
    return f'<path d="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{n(width)}" opacity="{n(opacity)}" stroke-linejoin="miter"/>'


def cut_rect(w, h, cut=20, inset=9, notch=False):
    cut = min(cut, (h - 2 * inset) / 2)
    points = [(inset, inset + cut), (inset + cut, inset)]
    if notch:
        points.extend([(w * .58, inset), (w * .60, inset + 6), (w * .72, inset + 6), (w * .74, inset)])
    points.extend([(w - inset - cut, inset), (w - inset, inset + cut), (w - inset, h - inset - cut),
                   (w - inset - cut, h - inset), (inset + cut, h - inset), (inset, h - inset - cut)])
    return path(points)


def star(cx, cy, rx, ry, waist=.14):
    a, b = rx * waist, ry * waist
    # Four curved tips, rather than a traced collection of aliased pixels.
    d = (f"M{n(cx)},{n(cy-ry)} "
         f"C{n(cx+a)},{n(cy-b)} {n(cx+a)},{n(cy-b)} {n(cx+rx)},{n(cy)} "
         f"C{n(cx+a)},{n(cy+b)} {n(cx+a)},{n(cy+b)} {n(cx)},{n(cy+ry)} "
         f"C{n(cx-a)},{n(cy+b)} {n(cx-a)},{n(cy+b)} {n(cx-rx)},{n(cy)} "
         f"C{n(cx-a)},{n(cy-b)} {n(cx-a)},{n(cy-b)} {n(cx)},{n(cy-ry)} Z")
    return f'<path d="{d}" fill="{PINK}"/>'


def defs(w, h, body):
    return f'''<defs>
  <linearGradient id="body" x1="0" y1="0" x2="1" y2="1" gradientUnits="objectBoundingBox">
   <stop stop-color="#240514"/><stop offset=".30" stop-color="#080507"/><stop offset=".70" stop-color="#080507"/><stop offset="1" stop-color="#32051b"/>
  </linearGradient>
  <linearGradient id="fill" x1="0" y1="0" x2=".45" y2="1"><stop stop-color="#15040d"/><stop offset=".45" stop-color="#49051f"/><stop offset="1" stop-color="{PINK}"/></linearGradient>
  <linearGradient id="fade"><stop stop-color="{PINK}"/><stop offset="1" stop-color="{PINK}" stop-opacity="0"/></linearGradient>
  <linearGradient id="sweep"><stop stop-color="{PINK}" stop-opacity="0"/><stop offset=".55" stop-color="#87013d" stop-opacity=".5"/><stop offset=".96" stop-color="#ff1684"/><stop offset="1" stop-color="#ffe7f4"/></linearGradient>
  <linearGradient id="cornerMask" x1="0" y1="0" x2="1" y2="1"><stop stop-color="white"/><stop offset=".29" stop-color="white" stop-opacity=".4"/><stop offset=".48" stop-color="black"/><stop offset=".62" stop-color="black"/><stop offset="1" stop-color="white"/></linearGradient>
  <pattern id="hatch" width="12" height="12" patternUnits="userSpaceOnUse" patternTransform="rotate(44)"><path d="M0,0 V12 M3,0 V12 M7,0 V12" fill="none" stroke="{PINK}" stroke-width=".55"/><path d="M0,4 H12 M0,10 H12" fill="none" stroke="#c40058" stroke-width=".42"/></pattern>
  <pattern id="fineHatch" width="7" height="7" patternUnits="userSpaceOnUse" patternTransform="rotate(-44)"><path d="M0,0 V7 M3,0 V7" fill="none" stroke="{PINK}" stroke-width=".42"/></pattern>
  <clipPath id="bodyClip"><path d="{body}"/></clipPath>
  <mask id="textureMask"><rect width="{w}" height="{h}" fill="url(#cornerMask)"/></mask>
  <filter id="glow" x="-10%" y="-25%" width="120%" height="150%" color-interpolation-filters="sRGB"><feGaussianBlur stdDeviation="1.3"/></filter>
 </defs>'''


def texture(w, h, seed, density=.50):
    rng = random.Random(seed)
    strokes = []
    # Short geometric scratches in the corners. No embedded noise bitmap.
    for _ in range(min(105, int(w * h / 900) + 24)):
        x, y = rng.uniform(4, w - 4), rng.uniform(4, h - 4)
        if .31 < (x / w + y / h) / 2 < .67:
            continue
        length = rng.uniform(5, min(95, max(12, w * .17)))
        strokes.append(line([(x, y), (x + length, y + length)], rng.choice([.4, .65, 1]), opacity=rng.uniform(.08, .34)))
    return (f'<g id="texture" clip-path="url(#bodyClip)"><g mask="url(#textureMask)">'
            f'<rect width="{w}" height="{h}" fill="url(#hatch)" opacity="{density}"/>'
            f'<rect width="{w}" height="{h}" fill="url(#fineHatch)" opacity=".30"/>'
            + "".join(strokes) + '</g></g>')


def edge(w, h, d, heavy=2.5):
    return ('<g id="glow-layer" opacity=".42" filter="url(#glow)">' + shape(d, width=heavy + 1) + '</g>'
            + '<g id="outline">' + shape(d, "url(#body)", width=heavy) + '</g>')


def slashes(x, y, count=3, step=18, width=12, height=20):
    return "".join(poly([(x+i*step, y+height), (x+i*step+height, y),
                         (x+i*step+height+width, y), (x+i*step+width, y+height)]) for i in range(count))


def corners(w, h, strong=False):
    c = min(34, h * .28)
    top = poly([(9, c+13), (c+13, 9), (c+46, 9), (c+11, 17), (17, c+11), (17, h*.30 if strong else c+27)])
    bottom = poly([(w-9, h-c-13), (w-c-13, h-9), (w-c-55, h-9), (w-c-11, h-18), (w-18, h-c-11), (w-18, h*.65)])
    return top + bottom


def framed(w, h, style, seed):
    c = min(28, h*.24)
    d = cut_rect(w, h, c, 10, style in ("main", "ornate", "capped", "section"))
    if style == "tabbed":
        tab = w*.41
        d = path([(10, 43), (38, 15), (tab-18, 15), (tab, 31), (w-18, 31),
                  (w-10, 39), (w-10, h-24), (w-24, h-10), (23, h-10), (10, h-23)])
    if style in ("strip-left", "strip-right"):
        d = cut_rect(w, h, 8, 9)
    if style == "footer":
        d = path([(72, 10), (w*.49, 10), (w*.52, 31), (w-28, 31), (w-10, 49), (w-10, h-22), (w-25, h-10), (60, h-10), (112, 18)])
    if style == "slanted":
        d = path([(9, h-19), (39, 10), (w-20, 10), (w-10, 20), (w-39, h-10), (18, h-10)])
    if style == "progress":
        d = path([(9, 12), (w-19, 12), (w-9, 20), (w-9, h-12), (w-21, h-8), (10, h-8)])
    if style.startswith("slot-"):
        variant = int(style[-1]); cut = [13, 8, 23, 0, 15, 13][variant]
        d = cut_rect(w, h, cut, 11)
    s = edge(w, h, d, 3 if style in ("header", "main", "section", "filled") else 2.1)
    if style in ("filled", "compact-panel"):
        s += f'<path id="fill-layer" d="{d}" fill="url(#fill)" opacity=".64"/>'
    density = .68 if style in ("main", "slot-0", "slot-3") else .34
    s += texture(w, h, seed, density)
    detail = []
    if style == "header":
        detail += [corners(w,h,True), slashes(21, 16, 3, 18, 10, 24),
                   slashes(w-128,h-34,4,18,12,23),
                   line([(44,16),(w-88,16)],1.1,LIGHT,.75),
                   line([(14,86),(39,109),(55,109)],2.5),
                   line([(w-19,38),(w-43,16),(w-84,16)],3),
                   poly([(15,58),(53,20),(90,20),(44,66),(60,83),(51,95)]),
                   poly([(w-13,55),(w-54,16),(w-91,16),(w-42,67),(w-60,85),(w-48,97)])]
    elif style == "main":
        detail += [corners(w,h,True),
                   poly([(12,h-115),(12,h-44),(44,h-12),(82,h-12),(54,h-27),(25,h-53),(25,h-139)]),
                   poly([(w-12,h-110),(w-12,h-39),(w-44,h-12),(w-92,h-12),(w-41,h-65),(w-38,h-89)]),
                   slashes(w-135,h-29,3,24,15,19),
                   line([(26,53),(53,24),(w*.33,24)],1,LIGHT,.8),
                   line([(w-25,75),(w-25,h-57),(w-55,h-27)],1,LIGHT,.8)]
    elif style == "tabbed":
        detail += [poly([(13,41),(38,16),(107,16),(78,42)]),
                   line([(14,46),(w*.39,46),(w*.42,31)],2.3),
                   poly([(10,54),(21,43),(21,88),(10,101)]),
                   slashes(w-100,h-49,3,23,13,39),
                   line([(31,h-17),(w-75,h-17)],1,LIGHT,.7)]
    elif style.startswith("slot-"):
        variant=int(style[-1])
        if variant==0:
            detail += [poly([(10,34),(28,11),(54,11),(22,42),(18,65)]),
                       poly([(w-11,h-42),(w-11,h-17),(w-20,h-11),(w-44,h-11),(w-21,h-37)]),
                       line([(17,50),(17,h-29),(29,h-17)],1)]
        elif variant==1:
            detail += [line([(16,28),(22,16),(w-23,16)],1.2,LIGHT),
                       line([(w-16,29),(w-16,h-27),(w-29,h-16),(w-44,h-16)],2.5),
                       poly([(12,22),(25,12),(34,12),(22,20),(15,33)]),
                       poly([(w-12,19),(w-31,12),(w-41,12),(w-23,22)])]
        elif variant==2:
            detail += [poly([(11,41),(19,18),(39,11),(w-34,11),(w-16,25),(w-12,44),(w-32,22),(37,20),(20,38)]),
                       poly([(11,h-40),(21,h-19),(38,h-11),(w-34,h-11),(w-14,h-26),(w-11,h-43),(w-31,h-21),(36,h-21),(20,h-38)])]
        elif variant==3:
            detail += [line([(16,16),(w-16,16)],1,LIGHT,.8), line([(16,h-16),(w-16,h-16)],1.2),
                       poly([(w-27,12),(w-12,12),(w-12,27)]), poly([(12,h-27),(12,h-12),(27,h-12)])]
        elif variant==4:
            detail += [line([(16,38),(16,24),(30,16),(w-27,16)],1.1,LIGHT),
                       slashes(w-58,h-47,3,13,7,34),
                       line([(15,h-32),(26,h-17),(39,h-17)],1.3)]
        else:
            detail += [line([(16,39),(16,26),(30,16),(w-30,16)],1.1),
                       line([(w-16,h-45),(w-16,h-25),(w-27,h-16),(w-44,h-16)],1.2),
                       poly([(w-12,h-32),(w-12,h-20),(w-25,h-12),(w-38,h-12)])]
    elif style == "footer":
        detail += [slashes(8,11,3,27,17,h-23),
                   line([(126,16),(w*.48,16),(w*.51,37),(w-40,37)],1,LIGHT,.65),
                   poly([(w-26,31),(w-10,48),(w-10,h-22),(w-25,h-10),(w-35,h-10),(w-17,h-27),(w-17,52),(w-37,32)])]
    elif style == "ornate":
        detail += [corners(w,h),poly([(w*.49,10),(w*.63,10),(w*.61,18),(w*.51,18)]),
                   slashes(12,14,2,10,6,12),slashes(w-48,h-28,2,10,6,12)]
    elif style == "capped":
        detail += [poly([(11,31),(30,11),(55,11),(28,38),(30,h-28),(54,h-11),(30,h-11),(11,h-31)]),
                   line([(54,h-16),(w-30,h-16),(w-16,h-30)],1,LIGHT,.75),
                   poly([(w-27,11),(w-11,27),(w-11,h-28),(w-27,h-11),(w-38,h-11),(w-22,h-31),(w-22,31)])]
    elif style == "section":
        detail += [corners(w,h),line([(17,39),(36,17),(79,17)],3),
                   line([(29,h-19),(w*.43,h-19),(w*.45,h-26)],1.3),
                   poly([(11,40),(11,69),(27,83),(32,76),(21,63),(21,36)]),
                   poly([(w-11,41),(w-11,67),(w-30,88),(w-46,88),(w-21,61),(w-21,33)])]
    elif style in ("strip-left","strip-right"):
        detail += [line([(17,14),(w-36,14)],.85,LIGHT,.65)]
        if style=="strip-left":detail += [slashes(w-47,h-23,3,10,6,15)]
        else:detail += [poly([(w-29,10),(w-10,26),(w-23,h-10),(w-36,h-10),(w-20,25)]),line([(w-18,h-12),(w-9,h-5)],.9)]
    elif style == "filled":
        detail += [corners(w,h),line([(14,37),(36,15),(w-37,15),(w-15,37),(w-15,h-38)],1.2,LIGHT),
                   poly([(w-11,h-42),(w-11,h-27),(w-28,h-11),(w-75,h-11),(w-48,h-24),(w-28,h-24)]),
                   poly([(14,h-46),(32,h-28),(43,h-31),(59,h-12),(31,h-12),(13,h-31)])]
    elif style == "compact":
        detail += [corners(w,h),line([(27,14),(w-23,14),(w-14,23)],1,LIGHT,.8)]
    elif style == "chevron":
        detail += [slashes(9,15,2,16,9,27),
                   poly([(w*.41,10),(w-25,10),(w-12,23),(w*.36,23)]),
                   line([(14,63),(14,37),(35,16),(89,16)],1.5,LIGHT),
                   poly([(w-11,h-45),(w-11,h-27),(w-28,h-11),(w-68,h-11),(w-42,h-23),(w-25,h-23)])]
    elif style == "compact-panel":
        detail += [corners(w,h),line([(16,35),(34,16),(w-33,16),(w-16,33),(w-16,h-31)],1,LIGHT,.8)]
    elif style == "tag":
        detail += [line([(16,30),(16,20),(25,14),(w-23,14)],1,LIGHT),line([(15,h-21),(24,h-15),(36,h-15)],1.2)]
    elif style == "slanted":
        detail += [line([(19,h-21),(41,16),(w-24,16)],.8,LIGHT,.65),poly([(w-23,11),(w-11,21),(w-38,h-11),(w-54,h-11)])]
    elif style == "corner":
        detail += [poly([(10,30),(29,10),(54,10),(24,40),(35,h-13),(20,h-13),(10,h-25)]),
                   line([(17,h-26),(26,h-17),(39,h-17)],2.5),line([(62,14),(w-14,14),(w-14,h-29)],.8,LIGHT,.7)]
    elif style == "progress":
        detail += [poly([(w-36,12),(w-18,12),(w-10,20),(w-10,h-13),(w-20,h-8),(w-38,h-8),(w-28,23)]),
                   poly([(w-25,17),(w-19,21),(w-25,27)],"#4a0423"),line([(14,16),(w-43,16)],.8,LIGHT,.8)]
    elif style == "slim":
        detail += [corners(w,h),line([(24,15),(w-24,15),(w-15,24)],.8,LIGHT,.6)]
    elif style == "lower-panel":
        detail += [poly([(11,h-46),(31,h-25),(54,h-11),(39,h-11),(12,h-28)]),
                   slashes(15,h-35,3,12,7,22),line([(17,35),(34,16),(w-16,16)],1,LIGHT,.7)]
    clip = '' if style == 'footer' else ' clip-path="url(#bodyClip)"'
    s += '<g id="accents"' + clip + '>' + "".join(detail) + '</g>'
    return d, s


def decorative(w,h,style,seed):
    s=[];d=cut_rect(w,h,4,8)
    if style=="rail":
        s += [poly([(9,23),(23,12),(w-29,12),(w-9,28),(w-9,h-10),(w-39,h-10),(w-64,19),(53,19),(33,23)]),
              poly([(52,14),(w-88,14),(w-83,18),(47,18)],"#090508"),
              slashes(w-137,19,4,13,5,17)]
    elif style=="star-rail":
        cy=h*.52;cx=w*.275
        s += [line([(10,cy),(w-12,cy),(w-12,cy+12),(w-38,h-12),(w*.35,h-12)],2.1),
              line([(16,cy+5),(cx-34,cy+5)],1),star(cx,cy,39,h*.43),
              poly([(w-107,cy+3),(w-13,cy+3),(w-13,cy+11),(w-40,h-13),(w-139,h-13)],"url(#fineHatch)",.7)]
    elif style=="notch-line":
        cy=h*.38;s += [line([(8,cy),(w*.39,cy),(w*.42,cy+6),(w*.58,cy+6),(w*.61,cy),(w-8,cy)],1.4),
                     line([(w*.07,cy+2),(w*.38,cy+2)],.6,LIGHT,.6)]
    elif style.startswith("orb-"):
        cx,cy=w/2,h/2;r=min(w,h)/2-11
        d=f'M{n(cx-r)},{n(cy)} A{n(r)},{n(r)} 0 1 0 {n(cx+r)},{n(cy)} A{n(r)},{n(r)} 0 1 0 {n(cx-r)},{n(cy)} Z'
        s += [f'<circle cx="{cx}" cy="{cy}" r="{r}" fill="#0d0409" stroke="{PINK}" stroke-width="1.35"/>']
        if style=="orb-reticle":
            s += [f'<circle cx="{cx}" cy="{cy}" r="{r-3}" fill="none" stroke="{PINK}" stroke-width=".6"/>',star(cx,cy,r*.68,r*.70,.12),
                  line([(cx,cy-r-4),(cx,cy-r+8)],.8),line([(cx,cy+r-7),(cx,cy+r+4)],.8),
                  line([(cx-r-4,cy),(cx-r+8,cy)],.8),line([(cx+r-8,cy),(cx+r+4,cy)],.8),
                  f'<path d="M{n(cx-r+2)},{n(cy-12)} A{r-1},{r-1} 0 0 1 {n(cx+18)},{n(cy-r+5)}" fill="none" stroke="{PINK}" stroke-width="2.5"/>']
        elif style=="orb-segmented":
            s += [f'<g clip-path="url(#bodyClip)">'+poly([(cx,cy),(cx,cy-r),(cx-r,cy-r),(cx-r,cy)],"url(#fill)")
                  +poly([(cx,cy),(cx+r,cy),(cx+r,cy+r),(cx,cy+r)],"url(#fill)",.65)+'</g>',
                  line([(cx,cy-r),(cx,cy+r)],.5,PINK,.6),line([(cx-r,cy),(cx+r,cy)],.5,PINK,.5),
                  f'<path d="M{n(cx-r)},{cy} A{r},{r} 0 0 1 {cx},{n(cy-r)}" stroke="{PINK}" fill="none" stroke-width="2.4"/>']
        else:
            rng=random.Random(seed)
            s += [f'<g clip-path="url(#bodyClip)"><rect width="{w}" height="{h}" fill="url(#fineHatch)"/>']
            for _ in range(55):
                x,y=rng.uniform(10,w-10),rng.uniform(10,h-10);length=rng.uniform(3,17)
                s.append(line([(x,y),(x+length*.7,y+length)],rng.uniform(.5,1.8),opacity=rng.uniform(.4,1)))
            s += ['</g>']
    elif style in ("sparkle","cross"):
        s += [star(w/2,h/2,w/2-9,h/2-9,.19 if style=="sparkle" else .085)]
    elif style=="double-star":
        cy=h*.55;s += [line([(36,cy-11),(w-16,cy-11),(w-10,cy-4),(w-30,cy+12),(43,cy+12)],1.4),
                       poly([(w*.53,cy-9),(w-17,cy-9),(w-12,cy-3),(w-31,cy+10),(w*.42,cy+10)],"url(#fineHatch)",.5),
                       star(26,cy+9,17,22,.15),star(26,cy-14,11,17,.12),line([(43,h-9),(86,h-9)],.7)]
    elif style=="indicator":
        for i in range(12):
            x=10+i*(w-49)/11;opacity=1 if i<6 else (12-i)/7
            s += [poly([(x, h-9),(x+19,9),(x+31,9),(x+12,h-9)], PINK, opacity)]
            if i>=7:
                s += [poly([(x+1,h-10),(x+19,11),(x+28,11),(x+10,h-10)],"url(#fineHatch)",.8)]
    elif style=="dissolve":
        s += [f'<rect x="8" y="9" width="{w-16}" height="{h-18}" fill="url(#sweep)"/>']
        rng=random.Random(seed)
        for _ in range(115):
            x=rng.uniform(8,w*.48);y=rng.uniform(9,h-9);z=rng.uniform(.8,3.2)
            s += [poly([(x,y),(x+z*2,y-z),(x+z,y+z*2)],PINK,max(0,1-x/(w*.5))*.8)]
        s += [poly([(8,h-10),(8,h*.48),(25,h*.68),(39,h*.68),(44,h-10)])]
    elif style=="halftone":
        # Actual vector circles and holes, with continuous progression in density.
        holes=[];dots=[]
        for iy in range(9):
            for ix in range(37):
                x=8+ix*7+(iy%2)*3.5;y=9+iy*5.1
                if y>h-9 or x>w-8:continue
                t=max(0,min(1,(x-w*.27)/(w*.65)))
                radius=.3+3.6*t
                holes += [f'<circle cx="{n(x)}" cy="{n(y)}" r="{n(radius)}" fill="black"/>']
                if x>w*.61:dots += [f'<circle cx="{n(x)}" cy="{n(y)}" r="{n(max(.25,2.5*(1-t)))}" fill="{PINK}"/>']
        s += [f'<defs><mask id="dots"><rect width="{w}" height="{h}" fill="white"/>'+''.join(holes)+'</mask></defs>',
              f'<rect x="8" y="9" width="{w*.70}" height="{h-18}" fill="{PINK}" mask="url(#dots)"/>',''.join(dots)]
    elif style=="scratch":
        d=path([(8,h-10),(w-8,9),(w-8,h-10)])
        rng=random.Random(seed);s += ['<g clip-path="url(#bodyClip)">']
        for i in range(155):
            x=rng.uniform(-30,w);y=rng.uniform(0,h);length=rng.uniform(10,105)
            s += [line([(x,y),(x+length,y-length*.65)],rng.uniform(.35,1.6),opacity=rng.uniform(.25,1))]
        s += ['</g>']
    body=''.join(s)
    # Soft glow is a separate, editable group; hard geometry remains sharp.
    if style not in ("halftone","scratch","dissolve"):
        body='<g id="glow-layer" opacity=".24" filter="url(#glow)">'+body+'</g><g id="geometry">'+body+'</g>'
    return d,body


def make_asset(entry, index):
    slug,title,category,bounds,style,slices=entry
    x,y,x2,y2=bounds;w,h=x2-x,y2-y
    seed=7300+index
    if category in ("dividers","symbols","textures","indicators") and style!="progress":
        d,body=decorative(w,h,style,seed)
    else:
        d,body=framed(w,h,style,seed)
    svg=(f'<?xml version="1.0" encoding="UTF-8"?>\n'
         f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" fill="none" role="img" aria-labelledby="title desc">\n'
         f'<title id="title">{escape(title)}</title>\n'
         '<desc id="desc">Vector reconstruction of UI_Interface.png. Editable paths, gradients and patterns; transparent exterior; no raster content.</desc>\n'
         +defs(w,h,d)+body+'\n</svg>\n')
    output=OUT/"svg"/category/(slug+".svg");output.parent.mkdir(parents=True,exist_ok=True);output.write_text(svg,encoding="utf-8")
    return {"id":slug,"name":title,"category":category,"svg":f"svg/{category}/{slug}.svg",
            "png_4x":f"png/4x/{category}/{slug}.png","width":w,"height":h,
            "source_bounds":list(bounds),"stretch_insets":list(slices) if slices else None,
            "safe_content_insets":list(slices) if slices else None,
            "scaling":"nine-slice" if slices else "uniform"}


def main():
    raw=(ROOT/"asset"/"UI_Interface.png").read_bytes()
    if raw[:8]!=b'\x89PNG\r\n\x1a\n' or struct.unpack('>II',raw[16:24])!=(1774,887):
        raise RuntimeError("The source sheet changed; review the asset bounds before rebuilding.")
    OUT.mkdir(parents=True,exist_ok=True)
    catalog={"version":1,"source":"../UI_Interface.png","source_size":[1774,887],"method":"Manual geometric reconstruction with procedural vector textures",
             "palette":{"neon":PINK,"highlight":LIGHT,"surface":"#080507"},"assets":[make_asset(a,i) for i,a in enumerate(ASSETS)]}
    (OUT/"catalog.json").write_text(json.dumps(catalog,indent=2)+"\n",encoding="utf-8")
    print(f"Created {len(catalog['assets'])} standalone SVG assets in {OUT / 'svg'}")


if __name__=="__main__":
    main()
