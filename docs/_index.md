---
title: 'Tree-sitter Deep Dive'
date: 2024-01-27T13:21:21+01:00
draft: true
---

[Tree-sitter](https://tree-sitter.github.io/tree-sitter/) is a parser generator
tool and an incremental parsing library. It can build a concrete syntax tree
for a source file and efficiently update the syntax tree as the source file is
edited.

Tree-sitter defines a standard library to implement grammars, with primitive
functions like `seq`, `repeat`, `choice`, etc. It has wide support for large
number of languages, setting a common standard for expression and tooling.

Real life applications include:
- core part of [neovim]()
- code highlights for various languages: [neovim-themes]()
- simple refactoring, snippets: [luasnip]()
- api navigation: [aerial]()

Further applications could include:
- formatting: Similar to highlights config, format config can be defined
- querying: CSS-like queries over codebase
- **code generation**: This is our goal.

## Table of contents

### Examples

### Real world

### Potential

### Little grammars
