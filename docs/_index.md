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
functions like `seq`, `repeat`, `choice`, etc. While LLMs use a linear
dependence, the above primitives dictate trees.
{{< sidenote 1 >}} With the
recent success of binary and tertiary GPT architectures, the weight matrix is
reduced to an adjacency matrix, in effect a graph (undirected for binary and
directed for tertiary case). This likely means that the operation: $ y_{[m]} =
A_{[m,n]}x_{[n]} $ simply reduces $y_i$ to an aggregate all nodes $x_j$
connected to it. {{< /sidenote >}}

- Goal: code generation
- Experiments: [Little grammars](/projects/little-grammars)
- Motivations
  - understand syntax
  - help with code generation / analysis
  - one language => most languages
- Questions
  - Can we implement `tree-sitter parse`, `tree-sitter query` and `tree-sitter highlight` in `c`?
  - Can we use tree-sitter to define `css` type selectors?
  - Can we translate tree-sitter query language to `sql`-like language?

