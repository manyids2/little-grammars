---
title: "Introduction"
draft: true
date: 2024-01-01T08:00:00-09:00
---

Tree-sitter defines a standard way of expressing the grammar of a language. It
supports 112 languages officially, with `markdown-inline` counting as 113,
whose properties are summarized here.

The properties of the language are defined using `grammar.js` file, which on
building using `tree-sitter generate` creates `grammar.json` and
`node-types.json` to parse `grammar.js` to standard form.

Grammar assumed to be in form of treesitter `grammar.json`
([The Grammar DSL](https://tree-sitter.github.io/tree-sitter/creating-parsers#the-grammar-dsl)).

Structure of `grammar.json`:

- `name`: name of language
- `word`: for keyword extraction optimization
- `rules`: dict of rules, keyed by name
- `extras`: with `type` (`SYMBOL`, `PATTERN`) and `name/value` (`comment`, some regex, etc.)
- `conflicts`: list of conflicts, each conflict is a list of rules
- `precedences`: havent seen any yet ( is it same as PREC? )
- `externals`: list of dicts with keys `type` (`SYMBOL`) and `name` ( acts like rule? )
- `inline`: list of rules
- `supertypes`: list of rules
- `PREC`: dict of rules and their priorities

After running the data collection in [data](#data), we find the following
tables in the `db`:

| name        | rows | cols |
| ----------- | ---: | ---: |
| languages   |   82 |   13 |
| rules       | 9693 |   10 |
| extras      |  195 |    5 |
| externals   |  383 |    5 |
| conflicts   |  686 |   12 |
| precedences |   83 |    4 |
| inlines     |  351 |    3 |
| supertypes  |  148 |    3 |
| precs       |   73 |    4 |

So, out of 113 languages, 82 of them managed to parse `grammar.json` correctly.

TODO: Check why the rest failed.

There are about ~10k rules with all languages combined, but the distribution
is very skewed. Let us check all the distributions by language.

<hr>

## Grammar

### rules

Rules are defined as javascript functions which look like `rule_c = ($) =>
seq(choice($.rule_a, $.rule_b))`, where `$` is a dictionary with all the rules.

A complete list of built-in functions that can be used in `grammar.js` to define rules is:

- Symbols ( the `$` object )
- Patterns ( string and regex )
- Sequences : seq(rule1, rule2, ...)
- Alternatives : choice(rule1, rule2, ...)
- Repetitions : repeat(rule)
- Options : optional(rule)
- Precedence : prec(number, rule)
- Left Associativity : prec.left([number], rule)
- Right Associativity : prec.right([number], rule)
- Dynamic Precedence : prec.dynamic(number, rule)
- Tokens : token(rule)
- Immediate Tokens : token.immediate(rule)
- Aliases : alias(rule, name)
- Field Names : field(name, rule)

The languages with most rules are:

| language            | count | percent |
| ------------------- | ----: | ------: |
| verilog             |   704 |   7\.26 |
| vhdl                |   427 |   4\.41 |
| pascal              |   312 |   3\.22 |
| cuda                |   286 |   2\.95 |
| c\-sharp            |   280 |   2\.89 |
| sql\-bigquery       |   280 |   2\.89 |
| cpp                 |   279 |   2\.88 |
| objc                |   260 |   2\.68 |
| qmljs               |   259 |   2\.67 |
| ssh\-client\-config |   236 |   2\.43 |
| sqlite              |   228 |   2\.35 |

As expected, `cpp` is close to the top. Unexpectedly, `sqlite` has many rules as well.
A comparizon of the 'popular' languages and some easter eggs:

| language           | count | percent |
| ------------------ | ----: | ------: |
| cpp                |   279 |   2\.88 |
| rust               |   173 |   1\.78 |
| java               |   170 |   1\.75 |
| c                  |   167 |   1\.72 |
| python             |   148 |   1\.53 |
| javascript         |   141 |   1\.45 |
| julia              |   125 |   1\.29 |
| go                 |   116 |   1\.20 |
| zig                |   106 |   1\.09 |
| bash               |    99 |   1\.02 |
| css                |    58 |   0\.60 |
| pgn                |    50 |   0\.52 |
| fish               |    49 |   0\.51 |
| regex              |    34 |   0\.35 |
| gitignore          |    23 |   0\.24 |
| query              |    23 |   0\.24 |
| html               |    19 |   0\.20 |
| json               |    13 |   0\.13 |
| comment            |     8 |   0\.08 |
| embedded\-template |     7 |   0\.07 |
| sexp               |     4 |   0\.04 |

DISCLAIMER: The following observations must be taken with a pinch of salt as:

- number of rules may not be a measurement of complexity
- implementation of rules may differ
- some languages may be easier to express with the tree-sitter protocol

That being said, let us jump straight to conclusions:

- As a user of `fish` shell, this is QED level proof that it is almost exactly
  twice as good as `bash`.
- As expected, `cpp` is close to the top. I told you so.
- `cpp` is almost twice as complicated as `c`.
- `javascript` and `python` are in the same ballpark.
- `go` and `zig` have less rules than `python` and `javascript`, but `rust` is more complicated.
- `html` is not much more complicated than `json`. `gitignore` has more rules than `html`.
- `pgn` is chess => there are 50 rules of chess?
- `regex`. Nuff said.
- The simplest is `sexp` which looks like this: `(source (file (as) (s_expression)))`.

It is a bit unexpected though that there are 9.7k rules. Even after removing
duplicates, it comes down to 6.7k.

| n_duplicates | count | name       |
| -----------: | ----: | ---------- |
|           67 |     1 | comment    |
|           41 |     1 | identifier |
|           34 |     1 | string     |
|          ... |   ... |            |
|            5 |   108 |            |
|            4 |    86 |            |
|            3 |   133 |            |
|            2 |   578 |            |
|            1 |  5698 |            |

TODO: How would we deal with same rules with different names? (graph isomorphism?)

Some of the most common rules are:

| rule                     | count |
| ------------------------ | ----: |
| comment                  |    67 |
| identifier               |    41 |
| string                   |    34 |
| escape_sequence          |    33 |
| \_expression             |    26 |
| source_file              |    24 |
| \_statement              |    23 |
| binary_expression        |    23 |
| if_statement             |    22 |
| parenthesized_expression |    22 |
| while_statement          |    21 |
| for_statement            |    20 |
| ...                      |   <20 |

Curiously, languages that implement (`break_statement` and
`continue_statement`), and (`true` and `false`) share a large overlap ( `c`,
`cpp`, `cuda`, `glsl`, `go`, `hack`, `java`, `javascript`, `objc`, `python`,
`qmljs`, `wgsl` ).

The differing cases are:

| type             | 1        | 2     | 3     | 4          | 5             |
| ---------------- | -------- | ----- | ----- | ---------- | ------------- |
| `break-continue` | c\-sharp | julia | php   | sourcepawn | sql\-bigquery |
| `true-false`     | json5    | json  | proto | r          | ruby          |

Ok, go back a minute. Just to make sure my eyes are not deceiving me, and `c`
has `true` and `false`, lets check the `grammar.json`...

{{< figure label="c-true" src="/projects/tree-sitter/01-introduction/c-true-false-symbols.png" alt="c-true" width="80%" class="center" >}}
c-true-false-symbols
{{< /figure >}}

It is a indeed defined, and a member of the rule `_expression_not_binary`.
[Ok then. Today I learned](https://github.com/tree-sitter/tree-sitter-c/blob/212a80f86452bb1316324fa0db730cf52f29e05a/src/grammar.json#L5272).

Each rule is of one of the following types ([The Grammar
DSL](https://tree-sitter.github.io/tree-sitter/creating-parsers#the-grammar-dsl)):

| type            | count | percent |
| --------------- | ----: | ------: |
| SEQ             |  4489 |  46\.31 |
| CHOICE          |  1819 |  18\.77 |
| PATTERN         |   582 |   6\.00 |
| PREC_RIGHT      |   491 |   5\.07 |
| TOKEN           |   430 |   4\.44 |
| PREC            |   422 |   4\.35 |
| PREC_LEFT       |   389 |   4\.01 |
| STRING          |   279 |   2\.88 |
| ALIAS           |   256 |   2\.64 |
| SYMBOL          |   161 |   1\.66 |
| REPEAT1         |   113 |   1\.17 |
| PREC_DYNAMIC    |    98 |   1\.01 |
| IMMEDIATE_TOKEN |    66 |   0\.68 |
| FIELD           |    52 |   0\.54 |
| REPEAT          |    46 |   0\.47 |

If you were wondering, `REPEAT` - zero-or-more ; `REPEAT1` - one-or-more.
We will explore rules more in depth in a later chapter.

<hr>

### extras

An array of tokens that may appear anywhere in the language. This is often used
for whitespace and comments. The default value of extras is to accept
whitespace. To control whitespace explicitly, specify extras: $ => [] in your
grammar.

| language      | count | percent |
| ------------- | ----: | ------: |
| sourcepawn    |    16 |   8\.21 |
| objc          |     8 |   4\.10 |
| gleam         |     5 |   2\.56 |
| elixir        |     5 |   2\.56 |
| tablegen      |     4 |   2\.05 |
| sql\-bigquery |     4 |   2\.05 |
| ruby          |     4 |   2\.05 |
| fortran       |     4 |   2\.05 |
| bash          |     4 |   2\.05 |
| wgsl          |     3 |   1\.54 |
| vhdl          |     3 |   1\.54 |
| scss          |     3 |   1\.54 |
| scala         |     3 |   1\.54 |
| rust          |     3 |   1\.54 |
| python        |     3 |   1\.54 |
| ...           |       |         |

Most languages have <=5, `objc` has 8 and `sourcepawn` has 16. What are the 16
extras for `sourcepawn`? I have no idea what `sourcepawn` is, but it has a
preprocessor.

| language   | name           | type    | value            |
| ---------- | -------------- | ------- | ---------------- |
| sourcepawn |                | PATTERN | \\s\|\\\\\\r?\\n |
| sourcepawn | comment        | SYMBOL  |                  |
| sourcepawn | preproc_endif  | SYMBOL  |                  |
| sourcepawn | preproc_if     | SYMBOL  |                  |
| sourcepawn | preproc_elseif | SYMBOL  |                  |
| ...        | ...            | ...     |                  |

Extras can be of 3 types:

| type    | count | percent |
| ------- | ----: | ------: |
| SYMBOL  |   118 |  60\.51 |
| PATTERN |    76 |  38\.97 |
| ALIAS   |     1 |   0\.51 |

- `SYMBOL`s always have a name.
- `PATTERN`s and `ALIAS`es only have values

Most `PATTERN`s represent whitespace, so I guess it is a good repo of
whitespace patterns.

- `\t` : tab
- `\r` : return
- `\f` : ?
- `\v` : ?
- `\n` : newline
- `\s` : whitespace
- `\u` : unicode
- `\p` : some new age regex stuff
- `\x` : only in erlang

Also, what is `\uFEFF`?

| value                                                                                                                        | count | percent |
| ---------------------------------------------------------------------------------------------------------------------------- | ----: | ------: |
| \\s                                                                                                                          |    36 |  47\.37 |
| \\s\+                                                                                                                        |     7 |   9\.21 |
| \\s \| \\\\\\r?\\n                                                                                                           |     5 |   6\.58 |
| \[\\s\\f\\uFEFF\\u2060\\u200B\] \| \\\\\\r?\\n                                                                               |     3 |   3\.95 |
| \\r?\\n                                                                                                                      |     3 |   3\.95 |
| \[\\s\\p\{Zs\}\\uFEFF\\u2060\\u200B\]                                                                                        |     2 |   2\.63 |
| \\\\\\r?\\n                                                                                                                  |     2 |   2\.63 |
| \(\\s \| \\f\)                                                                                                               |     1 |   1\.32 |
| \[ \\f\\t\\v\\u00a0\\u1680\\u2000\-\\u200a\\u2028\\u2029\\u202f\\u205f\\u3000\\ufeff\]                                       |     1 |   1\.32 |
| \[ \\t\\r\\n\]                                                                                                               |     1 |   1\.32 |
| \[ \\t\]\+                                                                                                                   |     1 |   1\.32 |
| \[ \\t\] \| \\r?\\n \| \\\\\\r?\\n                                                                                           |     1 |   1\.32 |
| \[\\s\\f\\uFEFF\\u2060\\u200B\] \| \\r?\\n                                                                                   |     1 |   1\.32 |
| \[\\s\\p\{Zs\}\\uFEFF\\u2028\\u2029\\u2060\\u200B\]                                                                          |     1 |   1\.32 |
| \[\\s\\u00A0\\uFEFF\\u3000\]\+                                                                                               |     1 |   1\.32 |
| \[\\s\\uFEFF\\u0009\\u0020\\u000A\\u000D\]                                                                                   |     1 |   1\.32 |
| \[\\s\\uFEFF\\u2060\\u200B\\u00A0\]                                                                                          |     1 |   1\.32 |
| \[\\s\]                                                                                                                      |     1 |   1\.32 |
| \[\\u0009\-\\u000D\\u0085\\u2028\\u2029\\u0020\\u3000\\u1680\\u2000\-\\u2006\\u2008\-\\u200A\\u205F\\u00A0\\u2007\\u202F\]\+ |     1 |   1\.32 |
| \[\\x01\-\\x20\\x80\-\\xA0\]                                                                                                 |     1 |   1\.32 |
| \\\\\( \| \\t \| \\v \| \\f\)                                                                                                |     1 |   1\.32 |
| \\s\\n                                                                                                                       |     1 |   1\.32 |
| \\s \| \\\\n                                                                                                                 |     1 |   1\.32 |
| \\s \| \\r?\\n                                                                                                               |     1 |   1\.32 |
| \\u00A0 \| \\s \| \\\\\\r?\\n                                                                                                |     1 |   1\.32 |

Moving on, the only `ALIAS` is:

| language | type  | value |
| -------- | ----- | ----- |
| make     | ALIAS | \\    |

From `SYMBOL`s, most are named `comment`, and many of the rest have to do with
comments or preprocessor calls:

| name                | count |
| ------------------- | ----: |
| comment             |    58 |
| line_comment        |     6 |
| block_comment       |     4 |
| multiline_comment   |     3 |
| pragma              |     2 |
| line_continuation   |     2 |
| \_whitespace        |     2 |
| \_preprocessor_call |     1 |
| ...                 |     1 |

We will see how languages use `extras` in a later chapter.

<hr>

### externals

An array of token names which can be returned by an external scanner. External
scanners allow you to write custom C code which runs during the lexing process
in order to handle lexical rules (e.g. Python’s indentation tokens) that cannot
be described by regular expressions.

Externals again are of 3 types: `SYMBOL`, `PATTERN`, `STRING`,

| type    | count |
| ------- | ----: |
| SYMBOL  |   364 |
| STRING  |    18 |
| PATTERN |     1 |

The one `PATTERN` is from `bash`:

| language | type    | value |
| -------- | ------- | ----- |
| bash     | PATTERN | \\n   |

The `STRING`s are:

| language | value   |
| -------- | ------- |
| bash     | \}      |
| bash     | \]      |
| bash     | <<      |
| bash     | <<\-    |
| html     | /\>     |
| python   | \]      |
| python   | \)      |
| python   | \}      |
| python   | except  |
| qmljs    | \|\|    |
| ruby     | /       |
| scala    | else    |
| scala    | catch   |
| scala    | finally |
| scala    | extends |
| scala    | derives |
| scala    | with    |
| svelte   | /\>     |

Most of the `SYMBOL`s (36%) are from `markdown` and `rst`:

| language | count | percent |
| -------- | ----: | ------: |
| markdown |    94 |  25\.82 |
| rst      |    41 |  11\.26 |
| ruby     |    29 |   7\.97 |
| elixir   |    26 |   7\.14 |
| bash     |    22 |   6\.04 |
| julia    |    12 |   3\.30 |
| php      |    12 |   3\.30 |
| svelte   |    11 |   3\.02 |
| ...      |   <10 |         |

For example, some symbols from `markdown` and `rst`:

| language | name               |
| -------- | ------------------ |
| markdown | \_line_ending      |
| markdown | \_soft_line_ending |
| rst      | \_newline          |
| rst      | \_blankline        |

We will see how languages use `externals` in a later chapter.

<hr>

### conflicts

An array of arrays of rule names. Each inner array represents a set of rules
that’s involved in an LR(1) conflict that is intended to exist in the grammar.
When these conflicts occur at runtime, Tree-sitter will use the GLR algorithm
to explore all of the possible interpretations. If multiple parses end up
succeeding, Tree-sitter will pick the subtree whose corresponding rule has the
highest total dynamic precedence.

| language | count | percent |
| -------- | ----: | ------: |
| verilog  |   181 |  26\.38 |
| qmljs    |    68 |   9\.91 |
| c\-sharp |    48 |   7\.00 |
| vhdl     |    41 |   5\.98 |
| kotlin   |    25 |   3\.64 |
| fortran  |    22 |   3\.21 |
| objc     |    21 |   3\.06 |
| scala    |    21 |   3\.06 |
| cpp      |    20 |   2\.92 |
| ...      |   <20 |         |

Most conflicts are between two and three elements:

| n_conflicts | count |
| ----------: | ----: |
|           9 |     1 |
|           8 |     2 |
|           7 |     4 |
|           6 |     8 |
|           5 |    11 |
|           4 |    25 |
|           3 |    97 |
|           2 |   425 |
|           1 |   113 |

Yes, there are conflicts with only one member as well. In fact, it is the
second most common type of conflict. Three-ways only come third. World peace is
a lie.

<hr>

### inlines

An array of rule names that should be automatically removed from the grammar by
replacing all of their usages with a copy of their definition. This is useful
for rules that are used in multiple places but for which you don’t want to
create syntax tree nodes at runtime.

| language   | count | percent |
| ---------- | ----: | ------: |
| vhdl       |    81 |  23\.08 |
| verilog    |    47 |  13\.39 |
| qmljs      |    24 |   6\.84 |
| capnp      |    16 |   4\.56 |
| re2c       |    16 |   4\.56 |
| javascript |    15 |   4\.27 |
| hack       |    11 |   3\.13 |
| bash       |    10 |   2\.85 |
| cpp        |    10 |   2\.85 |
| cuda       |    10 |   2\.85 |
| php        |    10 |   2\.85 |
| ...        |   <10 |         |

Some examples are:

| rule                         | count | percent |
| ---------------------------- | ----: | ------: |
| \_statement                  |    11 |   3\.13 |
| \_type_identifier            |    11 |   3\.13 |
| \_field_identifier           |     9 |   2\.56 |
| \_top_level_item             |     9 |   2\.56 |
| \_block_item                 |     6 |   1\.71 |
| \_statement_identifier       |     5 |   1\.42 |
| \_non_case_statement         |     5 |   1\.42 |
| \_assignment_left_expression |     5 |   1\.42 |
| ...                          |    <5 |         |

We will explore inlines more in depth in a later chapter.

<hr>

### supertypes

An array of hidden rule names which should be considered to be ‘supertypes’ in
the generated node types file.

| language | count | percent |
| -------- | ----: | ------: |
| erlang   |    22 |  14\.86 |
| ruby     |    15 |  10\.14 |
| java     |    10 |   6\.76 |
| ...      |   <10 |         |

Some examples are:

| rule                  | count | percent |
| --------------------- | ----: | ------: |
| \_expression          |    16 |  10\.81 |
| \_statement           |    13 |   8\.78 |
| \_type                |     6 |   4\.05 |
| \_declarator          |     5 |   3\.38 |
| \_field_declarator    |     5 |   3\.38 |
| \_type_declarator     |     5 |   3\.38 |
| \_abstract_declarator |     5 |   3\.38 |
| expression            |     5 |   3\.38 |
| statement             |     4 |   2\.70 |

We will explore supertypes more in depth in a later chapter.

<hr>

### precedences

An array of array of strings, where each array of strings defines named
precedence levels in descending order. These names can be used in the prec
functions to define precedence relative only to other names in the array,
rather than globally. Can only be used with parse precedence, not lexical
precedence.

| language      | count | percent |
| ------------- | ----: | ------: |
| qmljs         |    36 |  43\.37 |
| vhdl          |    23 |  27\.71 |
| javascript    |     7 |   8\.43 |
| markdown      |     6 |   7\.23 |
| scala         |     3 |   3\.61 |
| cpp           |     2 |   2\.41 |
| cuda          |     2 |   2\.41 |
| org           |     2 |   2\.41 |
| sql\-bigquery |     1 |   1\.20 |
| sqlite        |     1 |   1\.20 |

Some examples are:

| language   | n_precedences | list_name_type_value                                                                                                                                                                                                                                                                                              |
| ---------- | ------------: | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| cpp        |             2 | argument_list, SYMBOL; type_qualifier, SYMBOL                                                                                                                                                                                                                                                                     |
| cuda       |             2 | argument_list, SYMBOL; type_qualifier, SYMBOL                                                                                                                                                                                                                                                                     |
| javascript |             2 | STRING, assign; primary_expression, SYMBOL                                                                                                                                                                                                                                                                        |
| markdown   |             2 | \_setext_heading1, SYMBOL; \_block, SYMBOL                                                                                                                                                                                                                                                                        |
| org        |             2 | STRING, document_directive; STRING, body_directive                                                                                                                                                                                                                                                                |
| sqlite     |            14 | STRING, unary_bitnot; STRING, unary_plus; STRING, expr_collate; STRING, binary_concat; STRING, binary_times; STRING, binary_plus; STRING, binary_bitwise; STRING, binary_compare; STRING, binary_relation; STRING, expr_not_exists; STRING, expr_exists; STRING, unary_not; STRING, binary_and; STRING, binary_or |
| vhdl       |             2 | STRING, declaration; STRING, primary_unit                                                                                                                                                                                                                                                                         |

We will explore precedences more in depth in a later chapter.

<hr>

### PRECs

Not defined on [The Grammar DSL](https://tree-sitter.github.io/tree-sitter/creating-parsers#the-grammar-dsl).

Probably defines precedence as well, but in numbers.

| language   | count | percent |
| ---------- | ----: | ------: |
| c          |    21 |  28\.77 |
| sourcepawn |    20 |  27\.40 |
| python     |    18 |  24\.66 |
| fortran    |    14 |  19\.18 |

For examples, the precs for `c`:

| rule             | prec |
| ---------------- | ---: |
| PAREN_DECLARATOR | \-10 |
| ASSIGNMENT       |  \-2 |
| CONDITIONAL      |  \-1 |
| DEFAULT          |    0 |
| LOGICAL_OR       |    1 |
| ...              |      |

We will explore PRECs more in depth in a later chapter.

## Nodes

`node-types.json` is just a list of nodes, as opposed to `grammar.json`, so is
easier to parse. However, like grammar rules, these nodes are also nested.

Each node has the following structure:

```python
class TypeNamed:
    type: str
    named: bool

class TypeNameValue:
    type: str
    name: Optional[str] = None
    value: Optional[str] = None

class NodeChild:
    multiple: bool
    required: bool
    types: list[TypeNameValue]

class NodeType:
    type: str
    named: bool
    fields: Optional[dict[str, NodeChild]] = None
    children: Optional[NodeChild] = None
    subtypes: Optional[list[TypeNamed]] = None
```

While there doesnt seem to be any recursion in the structs, `NodeChild.types`
is a list of `TypeNameValue`, which are references to nodes, thus making it
recursive.

Distribution across languages:

| language      | count | percent |
| ------------- | ----: | ------: |
| verilog       |  1011 |   6\.61 |
| wgsl          |   566 |   3\.70 |
| vhdl          |   528 |   3\.45 |
| sql           |   487 |   3\.18 |
| sql\-bigquery |   481 |   3\.15 |
| cpp           |   392 |   2\.56 |
| latex         |   385 |   2\.52 |
| rust          |   269 |   1\.76 |
| java          |   267 |   1\.75 |
| c             |   266 |   1\.74 |
| sqlite        |   256 |   1\.67 |
| javascript    |   225 |   1\.47 |
| python        |   218 |   1\.43 |
| zig           |   217 |   1\.42 |
| bash          |   184 |   1\.20 |
| go            |   183 |   1\.20 |
| css           |    97 |   0\.63 |
| fish          |    73 |   0\.48 |
| html          |    28 |   0\.18 |
| json          |    20 |   0\.13 |
| sexp          |     5 |   0\.03 |

DISCLAIMER: The following observations must be taken with a pinch of salt as:

- number of nodes may not be a measurement of complexity
- implementation of rules may differ, leading to differing number of node types
- some languages may be easier to express with the tree-sitter protocol

That being said, let us jump straight to conclusions:

- As a user of `fish` shell, this is QED level proof that it is way more than
  twice as good as `bash`.
- As expected, `cpp` is close to the top. I told you so.
- `cpp` is only 1.5 times as complicated as `c`.
- `javascript`, `python` and `zig` are in the same ballpark.
- `go` is slightly simpler than the above, but `rust` is more complicated.
- `html` is not much more complicated than `json`.
- The simplest is `sexp` which looks like this: `(source (file (as) (s_expression)))`.

The so called conclusions from `grammars` seem to more or less hold here as
well. That is to be expected, as nodes are a subset of rules (#TODO is it really?).

Nodes are of two types, named and not. They are evenly split.

| named | count | percent |
| ----: | ----: | ------: |
|     1 |  7798 |  50\.99 |
|     0 |  7494 |  49\.01 |

Examples of named nodes:

| type                     | count | percent |
| ------------------------ | ----: | ------: |
| comment                  |    69 |   0\.88 |
| identifier               |    40 |   0\.51 |
| escape_sequence          |    34 |   0\.44 |
| string                   |    31 |   0\.40 |
| source_file              |    23 |   0\.29 |
| binary_expression        |    23 |   0\.29 |
| if_statement             |    22 |   0\.28 |
| parenthesized_expression |    22 |   0\.28 |
| while_statement          |    21 |   0\.27 |
| for_statement            |    20 |   0\.26 |
| ...                      |   <20 |         |

Examples of unnamed nodes:

| type           | count | percent |
| -------------- | ----: | ------: |
| \)             |    67 |   0\.89 |
| \[             |    67 |   0\.89 |
| \]             |    67 |   0\.89 |
| \(             |    66 |   0\.88 |
| ,              |    66 |   0\.88 |
| :              |    65 |   0\.87 |
| for            |    27 |   0\.36 |
| &&             |    26 |   0\.35 |
| while          |    26 |   0\.35 |
| volatile       |    12 |   0\.16 |
| interface      |    12 |   0\.16 |
| async          |     9 |   0\.12 |
| package        |     9 |   0\.12 |
| foreach        |     7 |   0\.09 |
| \#define       |     6 |   0\.08 |
| \_\_inline     |     4 |   0\.05 |
| \_\_inline\_\_ |     4 |   0\.05 |
| ,@             |     3 |   0\.04 |
| CREATE         |     3 |   0\.04 |
| DROP           |     3 |   0\.04 |
| λ              |     2 |   0\.03 |

So, apparently, λ is an unnamed node (`agda`, `fennel`).

<hr/>

## Appendix: Data

- [languages.db](https://manyids2x.nl/languages)
- [nodes.db](https://manyids2x.nl/nodes)

<hr/>
