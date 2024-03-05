#!/bin/bash
YOUR_LANGUAGE_NAME="x"

set -euo pipefail

mkdir tree-sitter-"$YOUR_LANGUAGE_NAME"
cd tree-sitter-"$YOUR_LANGUAGE_NAME" || exit

# Install tree-sitter cli
# cargo install tree-sitter-cli
npm install tree-sitter-cli

# This will prompt you for input
npm init

# This installs a small module that lets your parser be used from Node
npm install --save nan

# This installs the Tree-sitter CLI itself
npm install --save-dev tree-sitter-cli

# In your shell profile script
echo "export PATH=$PATH:./node_modules/.bin" >>.envrc

echo "
module.exports = grammar({
  name: '$YOUR_LANGUAGE_NAME',

  rules: {
    // TODO: add the actual grammar rules
    source_file: \$ => 'hello'
  }
});
" >grammar.js

mkdir -p test/corpus
echo "
=====
hello
=====

hello

---

(source_file)
" >test/corpus/corpus.txt

tree-sitter generate
tree-sitter test
