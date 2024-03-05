#!/bin/bash
YOUR_LANGUAGE_NAME="x"

set -eo pipefail

# Ensure $1 exists
if [ -z "$1" ]; then
	echo "USAGE: run-experiment.sh FILENAME [filter]"
	exit 1
fi
current="$(pwd)"

out="output.txt"
if [ -z "$2" ]; then
	out="output.txt"
else
	out="output-$2.txt"
fi

# Make sure this follows experiment/run structure
src="$(realpath "$(dirname "$1")")"
if [ "$(pwd)" != "$(realpath "$src/../../..")" ]; then
	exit 1
fi
expt="$(basename "$(realpath "$src/..")")"
run="$(basename "$(realpath "$src")")"

# Copy to tree-sitter repo
dst="./tree-sitter-$YOUR_LANGUAGE_NAME"
cp "$src/grammar.js" "$dst/grammar.js"
cp "$src/corpus.txt" "$dst/test/corpus/corpus.txt"

# Generate
cd "$dst" || exit
if tree-sitter generate; then
	echo "Generated: $expt/$run"
else
	echo "Generate failed: "
	tree-sitter generate &>"$src/$out"
	exit 1
fi

# Test all or selected
temp="temp.txt"
echo "  expt  : $expt" >"$temp"
echo "  run   : $run" >>"$temp"
if [ -z "$2" ]; then
	tree-sitter test >>"$temp" || true
else
	tree-sitter test -f "$2" >>"$temp" || true
fi

# Diplay output with ani
cat "$temp"

# NOTE: Uncomment to clean up ansi characters to save
# sed -r "s/\x1B\[([0-9]{1,3}(;[0-9]{1,2};?)?)?[mGK]//g" "$temp" >"$out"
cp "$temp" "$out"
rm "$temp"

# Copy back results
cd "$current" || exit
cp "$dst/$out" "$src/$out"
cp "$dst/src/grammar.json" "$src/grammar.json"
cp "$dst/src/node-types.json" "$src/node-types.json"
