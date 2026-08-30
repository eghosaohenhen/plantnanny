#!/usr/bin/env python3
"""
png_to_rgb565.py

Batch-convert pixel-art PNGs into Adafruit_GFX-compatible C headers.
Point it at a folder of PNGs and an output folder; each foo.png becomes
foo.h with an RGB565 color array + a 1-bpp transparency mask.

Usage:
    python3 png_to_rgb565.py sprites/ headers/
    python3 png_to_rgb565.py sprites/ headers/ --alpha-threshold 100
    python3 png_to_rgb565.py one_sprite.png headers/     # single file also works

The output folder is created if it doesn't exist. On the ESP32 the const
arrays live in memory-mapped flash, so direct indexing (what drawRGBBitmap
does) works fine; PROGMEM is a harmless no-op.
"""

import argparse
import glob
import os
import sys
from PIL import Image


def rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def convert(path, alpha_threshold):
    img = Image.open(path).convert("RGBA")
    w, h = img.size
    px = img.load()

    colors = []
    opaque = []  # 1 = draw this pixel, 0 = transparent
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            colors.append(rgb565(r, g, b))
            opaque.append(1 if a >= alpha_threshold else 0)

    # Pack mask row-by-row, each row padded to a whole byte, MSB first.
    # Matches Adafruit_GFX::drawRGBBitmap(x, y, bitmap, mask, w, h).
    bw = (w + 7) // 8
    mask = []
    for y in range(h):
        for bx in range(bw):
            byte = 0
            for bit in range(8):
                byte <<= 1
                x = bx * 8 + bit
                if x < w and opaque[y * w + x]:
                    byte |= 1
            mask.append(byte)

    return w, h, colors, mask


def emit_array(name, values, ctype, per_line, fmt):
    out = [f"const {ctype} {name}[{len(values)}] PROGMEM = {{"]
    for i in range(0, len(values), per_line):
        chunk = ", ".join(fmt(v) for v in values[i:i + per_line])
        out.append("  " + chunk + ",")
    out.append("};")
    return "\n".join(out)


def c_symbol(stem):
    """Turn a filename stem into a valid C identifier."""
    s = "".join(ch if ch.isalnum() else "_" for ch in stem)
    if not s or s[0].isdigit():
        s = "_" + s
    return s


def build_header(path, alpha_threshold):
    stem = os.path.splitext(os.path.basename(path))[0]
    name = c_symbol(stem)
    w, h, colors, mask = convert(path, alpha_threshold)
    parts = [
        "#pragma once",
        "#include <stdint.h>",
        "#ifndef PROGMEM",
        "#define PROGMEM",
        "#endif",
        "",
        f"// {os.path.basename(path)}  ({w}x{h})",
        f"#define {name.upper()}_W {w}",
        f"#define {name.upper()}_H {h}",
        emit_array(f"{name}_data", colors, "uint16_t", 12, lambda v: f"0x{v:04X}"),
        emit_array(f"{name}_mask", mask, "uint8_t", 16, lambda v: f"0x{v:02X}"),
        "",
    ]
    return stem, "\n".join(parts)


def group_animations(stems):
    """Group frame stems by mood: 'happy_0','happy_1' -> {'happy':[...]}.
    Stems without a trailing _<number> become single-frame animations."""
    import re
    groups = {}
    for stem in stems:
        m = re.match(r"^(.*)_(\d+)$", stem)
        if m:
            mood, idx = m.group(1), int(m.group(2))
        else:
            mood, idx = stem, 0
        groups.setdefault(mood, []).append((idx, stem))
    for mood in groups:
        groups[mood].sort(key=lambda t: t[0])
    return groups


def build_manifest(stems, frame_ms, loop, emotions=False,
                   blink_close=120, blink_min=2500, blink_max=6000):
    groups = group_animations(stems)
    lines = ['#pragma once', '#include "sprite_player.h"', ""]
    for stem in sorted(stems):
        lines.append(f'#include "{stem}.h"')
    lines.append("")

    warnings = []
    for mood in sorted(groups):
        frames = groups[mood]           # list of (idx, stem), sorted by idx
        sym = c_symbol(mood)

        def frame_row(stem, comment=""):
            n = c_symbol(stem)
            tail = f"  // {comment}" if comment else ""
            return (f"  {{ {n}_data, {n}_mask, "
                    f"{n.upper()}_W, {n.upper()}_H }},{tail}")

        if emotions and len(frames) >= 2:
            # Emotion = blink: frame 0 eyes-open, frame 1 eyes-closed.
            lines.append(f"static const Frame {sym}_frames[] = {{")
            lines.append(frame_row(frames[0][1], "eyes open"))
            lines.append(frame_row(frames[1][1], "eyes closed"))
            lines.append("};")
            lines.append(
                f"const Animation ANIM_{sym.upper()} = {{\n"
                f"  {sym}_frames, 2, 0, true,\n"
                f"  ANIM_BLINK, {blink_close}, {blink_min}, {blink_max}\n"
                f"}};")
            if len(frames) > 2:
                warnings.append(f"{mood}: {len(frames)} frames, blink uses "
                                f"only _0 (open) and _1 (closed)")
        elif emotions:
            # Single frame in emotions mode: static, can't blink yet.
            lines.append(f"static const Frame {sym}_frames[] = {{")
            lines.append(frame_row(frames[0][1], "static (add a _1 closed frame to blink)"))
            lines.append("};")
            lines.append(
                f"const Animation ANIM_{sym.upper()} = "
                f"{{ {sym}_frames, 1, {frame_ms}, false }};")
            warnings.append(f"{mood}: only one frame, won't blink until a "
                            f"'{mood}_1' (eyes-closed) frame is added")
        else:
            # Plain frame sequence.
            lines.append(f"static const Frame {sym}_frames[] = {{")
            for _, stem in frames:
                lines.append(frame_row(stem))
            lines.append("};")
            loop_str = "true" if loop else "false"
            lines.append(
                f"const Animation ANIM_{sym.upper()} = "
                f"{{ {sym}_frames, {len(frames)}, {frame_ms}, {loop_str} }};")
        lines.append("")

    lines.append("// Table of every animation, for menus / iteration:")
    lines.append("struct NamedAnim { const char *name; const Animation *anim; };")
    lines.append("static const NamedAnim ALL_ANIMS[] = {")
    for mood in sorted(groups):
        sym = c_symbol(mood)
        lines.append(f'  {{ "{mood}", &ANIM_{sym.upper()} }},')
    lines.append("};")
    lines.append("")

    for w in warnings:
        print(f"  note: {w}", file=sys.stderr)
    return "\n".join(lines)


def collect_pngs(input_path):
    if os.path.isdir(input_path):
        return sorted(glob.glob(os.path.join(input_path, "*.png")) +
                      glob.glob(os.path.join(input_path, "*.PNG")))
    if os.path.isfile(input_path):
        return [input_path]
    return []


def main():
    ap = argparse.ArgumentParser(description="Batch PNG -> RGB565 C headers.")
    ap.add_argument("input", help="folder of PNGs (or a single .png)")
    ap.add_argument("output", help="output folder for .h files (created if missing)")
    ap.add_argument("--alpha-threshold", type=int, default=128,
                    help="alpha >= this is opaque (default 128)")
    ap.add_argument("--manifest", metavar="NAME",
                    help="also emit an animations header (e.g. animations.h) "
                         "grouping frames by <mood>_<n> filename")
    ap.add_argument("--frame-ms", type=int, default=150,
                    help="default per-frame duration in the manifest (default 150)")
    ap.add_argument("--no-loop", action="store_true",
                    help="manifest animations hold last frame instead of looping")
    ap.add_argument("--emotions", action="store_true",
                    help="treat each mood as a blink emotion: _0=eyes open, "
                         "_1=eyes closed")
    ap.add_argument("--blink-close-ms", type=int, default=120,
                    help="emotions: eyes-closed duration (default 120)")
    ap.add_argument("--blink-min-ms", type=int, default=2500,
                    help="emotions: shortest gap between blinks (default 2500)")
    ap.add_argument("--blink-max-ms", type=int, default=6000,
                    help="emotions: longest gap between blinks (default 6000)")
    args = ap.parse_args()

    pngs = collect_pngs(args.input)
    if not pngs:
        print(f"No PNGs found at: {args.input}", file=sys.stderr)
        sys.exit(1)

    os.makedirs(args.output, exist_ok=True)

    stems = []
    for path in pngs:
        stem, text = build_header(path, args.alpha_threshold)
        out_path = os.path.join(args.output, stem + ".h")
        with open(out_path, "w") as f:
            f.write(text)
        stems.append(stem)
        print(f"{os.path.basename(path):30s} -> {out_path}")

    if args.manifest:
        man_text = build_manifest(stems, args.frame_ms, not args.no_loop,
                                  emotions=args.emotions,
                                  blink_close=args.blink_close_ms,
                                  blink_min=args.blink_min_ms,
                                  blink_max=args.blink_max_ms)
        man_path = os.path.join(args.output, args.manifest)
        with open(man_path, "w") as f:
            f.write(man_text)
        print(f"{'(manifest)':30s} -> {man_path}")

    print(f"\nDone: {len(pngs)} header(s) written to {args.output}/", file=sys.stderr)


if __name__ == "__main__":
    main()