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
