#!/bin/sh
# Export actual UI drawing operations with explicitly simulated radio/model data.
set -eu
cd "$(dirname "$0")/.."
preview_dir=$(mktemp -d "${TMPDIR:-/tmp}/bitchat-previews.XXXXXX")
trap 'rm -rf "$preview_dir"' EXIT HUP INT TERM
crypto_dir=.pio/libdeps/m5stickc-plus2/Monocypher/src
${CC:-cc} -std=c99 -I "$crypto_dir" -c "$crypto_dir/monocypher.c" -o "$preview_dir/monocypher.o"
${CC:-cc} -std=c99 -I "$crypto_dir" -c "$crypto_dir/optional/monocypher-ed25519.c" -o "$preview_dir/ed25519.o"
${CC:-cc} -std=c99 -c .pio/libdeps/m5stickc-plus2/M5GFX/src/lgfx/utility/lgfx_miniz.c -o "$preview_dir/miniz.o"
${CXX:-c++} -std=c++17 -Wall -Wextra -Werror -I test/fakes -I include -I src -I "$crypto_dir" -DSTICK_BOARD_M5STICKC_PLUS2 \
  test/ui_preview.cpp test/fakes/HostBoard.cpp src/Feedback.cpp src/PeerTable.cpp src/PeerSelection.cpp src/Settings.cpp \
  src/Mesh.cpp src/Identity.cpp src/NoiseSession.cpp src/FragmentAssembler.cpp \
  src/WireCodec.cpp src/PacketSecurity.cpp src/CryptoPrimitives.cpp \
  "$preview_dir/monocypher.o" "$preview_dir/ed25519.o" "$preview_dir/miniz.o" -o "$preview_dir/preview"
"$preview_dir/preview" "$preview_dir"
python3 tools/render-previews.py "$preview_dir"
swift -module-cache-path "$preview_dir/swift-cache" tools/export-preview-jpegs.swift "$preview_dir" docs/screens
