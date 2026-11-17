#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
test_dir=$(mktemp -d "${TMPDIR:-/tmp}/bitchat-tests.XXXXXX")
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM
compiler=${CXX:-c++}
board_flag=-DSTICK_BOARD_M5STICKC_PLUS2
run_test() {
  name=$1
  shift
  "$compiler" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -I test/fakes -I include "$board_flag" "$@" -o "$test_dir/$name"
  "$test_dir/$name"
}
run_test profile test/board_profile_test.cpp src/boards/m5stickc-plus2/Profile.cpp
run_test clock test/clock_test.cpp src/Clock.cpp -DSTICK_CLOCK_DRY_RUN -DSTICK_BUILD_EPOCH=1700000000LL
run_test peers test/peer_table_test.cpp src/PeerTable.cpp
run_test navigation test/navigation_input_test.cpp
run_test composer test/message_composer_test.cpp
run_test joyc-navigation test/joyc_navigation_test.cpp
run_test joyc-driver test/joyc_driver_test.cpp src/Joyc.cpp
run_test feedback test/feedback_test.cpp src/Feedback.cpp
run_test settings test/settings_test.cpp src/Settings.cpp
run_test selection test/peer_selection_test.cpp src/PeerTable.cpp src/PeerSelection.cpp
run_test scanner test/scanner_test.cpp src/Scanner.cpp src/PeerTable.cpp

run_test codec test/wire_codec_test.cpp src/WireCodec.cpp

run_test fragments test/fragment_assembler_test.cpp src/FragmentAssembler.cpp src/WireCodec.cpp

crypto_dir=.pio/libdeps/m5stickc-plus2/Monocypher/src
if [ ! -f "$crypto_dir/monocypher.c" ]; then
  echo 'Install pinned dependencies first: pio pkg install -e m5stickc-plus2' >&2
  exit 1
fi
${CC:-cc} -std=c99 -fsanitize=address,undefined -I "$crypto_dir" -c "$crypto_dir/monocypher.c" -o "$test_dir/monocypher.o"
${CC:-cc} -std=c99 -fsanitize=address,undefined -I "$crypto_dir" -c "$crypto_dir/optional/monocypher-ed25519.c" -o "$test_dir/ed25519.o"
run_crypto_test() {
  name=$1
  shift
  "$compiler" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -I include -I "$crypto_dir" "$board_flag" "$@" "$test_dir/monocypher.o" "$test_dir/ed25519.o" -o "$test_dir/$name"
  if [ "$name" = packets ]; then
    "$test_dir/$name" "$test_dir/local-signatures.txt"
  else
    "$test_dir/$name"
  fi
}
run_crypto_test crypto test/crypto_primitives_test.cpp src/CryptoPrimitives.cpp

run_crypto_test noise test/noise_session_test.cpp src/NoiseSession.cpp src/CryptoPrimitives.cpp
${CC:-cc} -std=c99 -fsanitize=address,undefined -c .pio/libdeps/m5stickc-plus2/M5GFX/src/lgfx/utility/lgfx_miniz.c -o "$test_dir/miniz.o"
run_crypto_test packets test/packet_security_test.cpp src/PacketSecurity.cpp src/WireCodec.cpp src/CryptoPrimitives.cpp "$test_dir/miniz.o"

swift -module-cache-path "$test_dir/swift-cache" tools/VerifyPacketSignatures.swift "$test_dir/local-signatures.txt"

run_crypto_test identity test/identity_test.cpp src/Identity.cpp src/CryptoPrimitives.cpp -I test/fakes

run_crypto_test mesh test/mesh_test.cpp src/Mesh.cpp src/Identity.cpp src/NoiseSession.cpp src/FragmentAssembler.cpp src/WireCodec.cpp src/PacketSecurity.cpp src/CryptoPrimitives.cpp "$test_dir/miniz.o" -I test/fakes

run_crypto_test bot test/bot_test.cpp src/Mesh.cpp src/Identity.cpp src/NoiseSession.cpp src/FragmentAssembler.cpp src/WireCodec.cpp src/PacketSecurity.cpp src/CryptoPrimitives.cpp "$test_dir/miniz.o" -I test/fakes

run_crypto_test ui test/ui_test.cpp test/fakes/HostBoard.cpp src/Feedback.cpp src/PeerTable.cpp src/PeerSelection.cpp src/Settings.cpp src/Mesh.cpp src/Identity.cpp src/NoiseSession.cpp src/FragmentAssembler.cpp src/WireCodec.cpp src/PacketSecurity.cpp src/CryptoPrimitives.cpp "$test_dir/miniz.o" -I test/fakes -I src

run_crypto_test transport test/transport_test.cpp src/Mesh.cpp src/Identity.cpp src/NoiseSession.cpp src/FragmentAssembler.cpp src/WireCodec.cpp src/PacketSecurity.cpp src/CryptoPrimitives.cpp "$test_dir/miniz.o" -I test/fakes

core_files='src/main.cpp src/Clock.cpp src/Mesh.cpp src/Transport.cpp src/InfoBot.h src/Scanner.cpp src/Identity.cpp src/Settings.cpp include/*.h'
if grep -n -E 'M5StickC|M5Unified' $core_files; then
  echo 'Core sources still reference M5StickC or M5Unified' >&2
  exit 1
fi
