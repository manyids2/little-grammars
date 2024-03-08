---
title: "Tree-sitter Deep Dive"
date: 2024-01-29T13:21:21+01:00
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

- core part of [neovim](https://neovim.io/)
- code highlights for various languages: [neovim-themes](https://dotfyle.com/neovim/colorscheme/top)
- simple refactoring, snippets: [luasnip](https://github.com/L3MON4D3/LuaSnip)
- api navigation: [aerial](https://github.com/stevearc/aerial.nvim)

Further applications could include:

- format: Similar to highlights config, format config can be defined
- select: CSS-like queries over codebase, returning returning found text
- **code generation**: This is our goal.

## [Introduction](/projects/tree-sitter/01-introduction/01-index)

We look at how grammars are defined in tree-sitter by exploring implementations
for different languages.

Data:

- [languages.db](https://manyids2x.nl/languages)
- [nodes.db](https://manyids2x.nl/nodes)

## [Tools](/projects/tree-sitter/02-tools/02-index)

We try to implements tree-sitter's CLI tools: `parse`, `query` and `highlight`.

## [Potential]()

We propose new CLI tools: `select`, `format` and `layout`.

## [Little grammars](/projects/tree-sitter/04-little-grammars)

We use simple grammars as case studies to explore creating parsers.

- [barebones](./barebones/01-index)
- [s-expressions](./s-expressions/02-index)
- [valid-json](./valid-json/03-index)
- [xrandr](./xrandr/04-index)

<hr/>
