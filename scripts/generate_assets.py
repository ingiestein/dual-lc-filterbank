#!/usr/bin/env python3
"""Generate analog-style VSTGUI bitmaps and editor.uidesc."""

from __future__ import annotations

import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parents[1]
RES = ROOT / "resources"
GUI = ROOT / "src" / "gui"
RES.mkdir(parents=True, exist_ok=True)
GUI.mkdir(parents=True, exist_ok=True)

W, H = 980, 400
KNOB = 64
FRAMES = 80
CELL_LABELS = [
    "60 LP",
    "85",
    "120",
    "170",
    "240",
    "340",
    "480",
    "680",
    "960",
    "1.36k",
    "1.92k",
    "2.72k",
    "3.84k",
    "7.5k HP",
]


def font(size: int) -> ImageFont.ImageFont:
    for path in (
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    ):
        p = Path(path)
        if p.exists():
            return ImageFont.truetype(str(p), size)
    return ImageFont.load_default()


def font_bold(size: int) -> ImageFont.ImageFont:
    for path in (
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
    ):
        p = Path(path)
        if p.exists():
            return ImageFont.truetype(str(p), size)
    return font(size)


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(len(a)))


def make_background() -> Image.Image:
    img = Image.new("RGB", (W, H), (18, 16, 14))
    px = img.load()
    for y in range(H):
        for x in range(W):
            n = (math.sin(x * 0.035) + math.sin(y * 0.09) + math.sin((x + y) * 0.02)) / 3
            grain = ((x * 13 + y * 29) % 11) - 5
            base = 28 + int(8 * n) + grain
            if y < 54:
                px[x, y] = (min(70, base + 8), min(58, base - 2), min(40, base - 8))
            elif x >= 840:
                px[x, y] = (min(55, base + 4), min(46, base - 2), min(34, base - 6))
            else:
                px[x, y] = (max(16, base), max(14, base - 4), max(12, base - 8))

    img = img.filter(ImageFilter.SMOOTH)
    draw = ImageDraw.Draw(img, "RGBA")
    draw.rectangle([0, 0, W, 54], outline=(196, 165, 116, 180), width=1)
    draw.line([(0, 54), (W, 54)], fill=(212, 180, 122, 220), width=2)
    draw.line([(840, 54), (840, H)], fill=(196, 165, 116, 140), width=1)
    draw.rectangle([12, 64, 828, H - 16], outline=(90, 74, 52, 160), width=1)
    draw.rectangle([850, 64, 968, H - 16], outline=(90, 74, 52, 160), width=1)

    title = font_bold(22)
    sub = font(12)
    draw.text((22, 10), "DUAL-LC FILTERBANK", font=title, fill=(232, 220, 196, 255))
    draw.text(
        (22, 36),
        "constant-K inductor bank  ·  Q = 0.8  ·  parallel graphic",
        font=sub,
        fill=(196, 165, 116, 230),
    )
    draw.text((790, 16), "v1.0", font=sub, fill=(168, 140, 100, 220))

    # LC glyph
    cx, cy = 742, 28
    draw.arc([cx - 18, cy - 10, cx - 2, cy + 10], 200, 520, fill=(212, 180, 122, 255), width=2)
    draw.arc([cx - 6, cy - 10, cx + 10, cy + 10], 200, 520, fill=(212, 180, 122, 255), width=2)
    draw.line([(cx + 14, cy - 8), (cx + 14, cy + 8)], fill=(212, 180, 122, 255), width=2)
    draw.line([(cx + 20, cy - 8), (cx + 20, cy + 8)], fill=(212, 180, 122, 255), width=2)

    footer = font(11)
    draw.text(
        (24, H - 28),
        "cells ±12 dB   ·   preamp 0 to +24 dB   ·   output ±12 dB   ·   0 dBFS FS, operate near −18 dBFS",
        font=footer,
        fill=(150, 128, 96, 230),
    )
    return img.convert("RGB")


def make_knob_frame(angle_deg: float, accent: tuple[int, int, int]) -> Image.Image:
    img = Image.new("RGBA", (KNOB, KNOB), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img, "RGBA")
    cx = cy = KNOB / 2
    r = 28
    draw.ellipse([cx - r - 2, cy - r - 2, cx + r + 2, cy + r + 2], fill=(12, 10, 8, 220))
    for i in range(int(r), 8, -1):
        t = (r - i) / r
        col = lerp((58, 48, 36), (140, 118, 86), t * 0.7)
        draw.ellipse([cx - i, cy - i, cx + i, cy + i], fill=col + (255,))
    draw.ellipse([cx - 18, cy - 18, cx + 18, cy + 18], fill=(32, 26, 20, 255))
    draw.ellipse([cx - 16, cy - 16, cx + 16, cy + 16], fill=(48, 40, 30, 255))
    rad = math.radians(angle_deg)
    x2 = cx + 20 * math.sin(rad)
    y2 = cy - 20 * math.cos(rad)
    draw.line([(cx, cy), (x2, y2)], fill=accent + (255,), width=3)
    draw.ellipse([cx - 3, cy - 3, cx + 3, cy + 3], fill=(232, 210, 160, 255))
    return img


def make_knob_strip(accent: tuple[int, int, int], path: Path) -> None:
    strip = Image.new("RGBA", (KNOB, KNOB * FRAMES), (0, 0, 0, 0))
    start, sweep = -135.0, 270.0
    for i in range(FRAMES):
        ang = start + sweep * (i / (FRAMES - 1))
        frame = make_knob_frame(ang, accent)
        strip.paste(frame, (0, i * KNOB), frame)
    strip.save(path)


def make_snapshots(bg: Image.Image) -> None:
    uid = "B48F788026A44C3DAC806EF280802D49"
    small = bg.resize((128, 128), Image.Resampling.LANCZOS)
    large = bg.resize((256, 256), Image.Resampling.LANCZOS)
    small.save(RES / f"{uid}_snapshot.png")
    large.save(RES / f"{uid}_snapshot_2.0x.png")


def cell_origin(index: int) -> tuple[int, int]:
    col = index % 7
    row = index // 7
    x = 28 + col * 116
    y = 78 if row == 0 else 220
    return x, y


def view_knob(tag: str, x: int, y: int, default: float, tooltip: str, bitmap: str = "knob") -> str:
    return (
        f'        <view angle-range="270" angle-start="135" bitmap="{bitmap}" class="CAnimKnob" '
        f'control-tag="{tag}" default-value="{default}" height-of-one-image="{KNOB}" inverse-bitmap="false" '
        f'max-value="1" min-value="0" mouse-enabled="true" opacity="1" origin="{x}, {y}" size="{KNOB}, {KNOB}" '
        f'sub-pixmaps="{FRAMES}" tooltip="{tooltip}" transparent="true" value-inset="0" wants-focus="true" '
        f'wheel-inc-value="0.1" zoom-factor="1.5"/>'
    )


def view_label(x: int, y: int, w: int, title: str, color: str = "Gold") -> str:
    return (
        f'        <view back-color="Transparent" background-offset="0, 0" class="CTextLabel" default-value="0.5" '
        f'font="~ NormalFontSmall" font-antialias="true" font-color="{color}" frame-color="Transparent" '
        f'frame-width="0" max-value="1" min-value="0" mouse-enabled="false" opacity="1" origin="{x}, {y}" '
        f'round-rect-radius="0" shadow-color="~ BlackCColor" size="{w}, 16" style-3D-in="false" style-3D-out="false" '
        f'style-no-draw="false" style-no-frame="true" style-no-text="false" style-round-rect="false" '
        f'style-shadow-text="false" text-alignment="center" text-inset="0, 0" text-rotation="0" title="{title}" '
        f'transparent="true" value-precision="2" wants-focus="false" wheel-inc-value="0.1"/>'
    )


def view_param(tag: str, x: int, y: int) -> str:
    return (
        f'        <view back-color="ValueBack" background-offset="0, 0" class="CParamDisplay" control-tag="{tag}" '
        f'default-value="0.5" font="~ NormalFontSmall" font-antialias="true" font-color="Cream" frame-color="GoldDim" '
        f'frame-width="1" max-value="1" min-value="0" mouse-enabled="false" opacity="1" origin="{x}, {y}" '
        f'round-rect-radius="3" shadow-color="~ BlackCColor" size="64, 16" style-3D-in="false" style-3D-out="false" '
        f'style-no-draw="false" style-no-frame="false" style-no-text="false" style-round-rect="true" '
        f'style-shadow-text="false" text-alignment="center" text-inset="0, 0" text-rotation="0" transparent="false" '
        f'value-precision="1" wants-focus="false" wheel-inc-value="0.1"/>'
    )


def make_uidesc() -> str:
    views = []
    tags = []
    for i, label in enumerate(CELL_LABELS):
        tag = f"Cell{i}"
        tags.append(f'        <control-tag name="{tag}" tag="{i}"/>')
        x, y = cell_origin(i)
        kind = "low-pass" if i == 0 else ("high-pass" if i == 13 else "band-pass")
        views.append(view_knob(tag, x, y, 0.5, f"{label} {kind} gain ±12 dB"))
        views.append(view_label(x, y + 64, 64, label))
        views.append(view_param(tag, x, y + 80))

    tags.append('        <control-tag name="Preamp" tag="14"/>')
    tags.append('        <control-tag name="Output" tag="15"/>')
    views.append(view_knob("Preamp", 872, 86, 0.0, "Preamp 0 to +24 dB", "knob_amber"))
    views.append(view_label(860, 150, 88, "PREAMP", "Amber"))
    views.append(view_param("Preamp", 872, 168))
    views.append(view_knob("Output", 872, 220, 0.5, "Output ±12 dB", "knob_amber"))
    views.append(view_label(860, 284, 88, "OUTPUT", "Amber"))
    views.append(view_param("Output", 872, 302))

    body = "\n".join(views)
    tag_xml = "\n".join(tags)
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<vstgui-ui-description version="1">
    <colors>
        <color name="Gold" rgba="#c4a574ff"/>
        <color name="GoldDim" rgba="#8a704aff"/>
        <color name="Cream" rgba="#e8dcc8ff"/>
        <color name="Amber" rgba="#e0b060ff"/>
        <color name="Transparent" rgba="#00000000"/>
        <color name="ValueBack" rgba="#1a161080"/>
        <color name="Panel" rgba="#1c1814ff"/>
    </colors>
    <template background-color="Panel" background-color-draw-style="filled and stroked" bitmap="background" class="CViewContainer" mouse-enabled="true" name="view" opacity="1" origin="0, 0" size="{W}, {H}" transparent="false" wants-focus="false">
{body}
    </template>
    <bitmaps>
        <bitmap name="background" path="background.png"/>
        <bitmap name="knob" path="knob.png"/>
        <bitmap name="knob_amber" path="knob_amber.png"/>
    </bitmaps>
    <control-tags>
{tag_xml}
    </control-tags>
</vstgui-ui-description>
"""


def main() -> None:
    bg = make_background()
    bg.save(RES / "background.png")
    make_knob_strip((212, 180, 122), RES / "knob.png")
    make_knob_strip((232, 168, 72), RES / "knob_amber.png")
    make_snapshots(bg)
    (GUI / "editor.uidesc").write_text(make_uidesc(), encoding="utf-8")
    print("Wrote resources and src/gui/editor.uidesc")


if __name__ == "__main__":
    main()
