#!/usr/bin/env python3
"""Render the firmware UI with simulated data and refresh the website screenshots."""
from pathlib import Path
import json, re, subprocess, tempfile

root = Path(__file__).resolve().parents[3]
output = root / "website/public/screens"
crypto = root / ".pio/libdeps/m5stickc-plus2/Monocypher/src"
sources = [
    "Feedback",
    "PeerTable",
    "PeerSelection",
    "Settings",
    "Mesh",
    "Identity",
    "NoiseSession",
    "FragmentAssembler",
    "WireCodec",
    "PacketSecurity",
    "CryptoPrimitives",
]
with tempfile.TemporaryDirectory(prefix="bitchat-demo-") as directory:
    temp = Path(directory)
    for src, name in [
        (crypto / "monocypher.c", "monocypher"),
        (crypto / "optional/monocypher-ed25519.c", "ed25519"),
        (root / ".pio/libdeps/m5stickc-plus2/M5GFX/src/lgfx/utility/lgfx_miniz.c", "miniz"),
    ]:
        subprocess.run(["cc", "-std=c99", "-I", str(crypto), "-c", str(src), "-o", str(temp / f"{name}.o")], check=True)
    subprocess.run(
        [
            "c++",
            "-std=c++17",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-DSTICK_BOARD_M5STICKC_PLUS2",
            "-I",
            str(root / "test/fakes"),
            "-I",
            str(root / "include"),
            "-I",
            str(root / "src"),
            "-I",
            str(crypto),
            str(root / "test/ui_preview.cpp"),
            str(root / "test/fakes/HostBoard.cpp"),
            *[str(root / f"src/{name}.cpp") for name in sources],
            *[str(temp / f"{name}.o") for name in ("monocypher", "ed25519", "miniz")],
            "-o",
            str(temp / "preview"),
        ],
        check=True,
        cwd=root,
    )
    subprocess.run([str(temp / "preview"), str(temp)], check=True, cwd=root)
    subprocess.run(["python3", str(root / "tools/render-previews.py"), str(temp), "4"], check=True)
    keep = {
        "start",
        "home",
        "writing",
        "confirm",
        "queued",
        "received",
        "nearby",
        "channels",
        "infobot",
        "settings",
        "keyboard-sample",
        "home-nearby",
        "home-channels",
        "home-infobot",
        "home-settings",
        "reply-method-0",
        "reply-method-1",
        "reply-method-2",
    }
    for index in range(32):
        keep.update({f"short-{index}", f"review-{index}-cancel", f"review-{index}-send", f"queued-{index}"})
    kept = []
    for file in temp.glob("*.png"):
        if file.stem in keep:
            (output / file.name).write_bytes(file.read_bytes())
            kept.append(file.name)
    header = (root / "src/MessageComposer.h").read_text().split("presets[]={", 1)[1].split("};", 1)[0]
    presets = re.findall(r'"([^"]*)"', header)
    (root / "website/src/data/short-messages.json").write_text(json.dumps(presets, indent=2) + "\n")
    print(f"{len(kept)} landscape screenshots written to {output}.")
