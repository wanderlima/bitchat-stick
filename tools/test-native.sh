#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
test_dir=$(mktemp -d "${TMPDIR:-/tmp}/bitchat-tests.XXXXXX")
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM
compiler=${CXX:-c++}
run_test() {
  name=$1
  shift
  "$compiler" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -I test/fakes -I include "$@" -o "$test_dir/$name"
  "$test_dir/$name"
}
run_test peers test/peer_table_test.cpp src/PeerTable.cpp
run_test navigation test/navigation_input_test.cpp
run_test settings test/settings_test.cpp src/Settings.cpp
run_test selection test/peer_selection_test.cpp src/PeerTable.cpp src/PeerSelection.cpp
run_test scanner test/scanner_test.cpp src/Scanner.cpp src/PeerTable.cpp
run_test ui test/ui_test.cpp src/PeerTable.cpp src/PeerSelection.cpp src/Settings.cpp

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
  "$compiler" -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -I include -I "$crypto_dir" "$@" "$test_dir/monocypher.o" "$test_dir/ed25519.o" -o "$test_dir/$name"
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
