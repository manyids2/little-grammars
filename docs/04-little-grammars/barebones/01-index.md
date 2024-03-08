---
title: "Barebones"
draft: true
date: 2024-01-01T08:00:00-09:00
---

In this chapter, we establish a basic workflow to start defining new grammars.

<hr/>

## baseline

We start with the given example from the starter kit (`grammar.js`):

```javascript
module.exports = grammar({
  name: 'barebones',
  rules: {
    source_file: $ => 'hello'
  }
});
```

We can test the grammar with a corpus:

```bash
==========================
basic
==========================
hello
---

(source_file)
```

Using `tree-sitter test`, we get

```bash
  corpus:
    ✓ basic
```

This will be our basic flow to set up and test rules.

For example, to check what happens when we add newlines, we modify the corpus:

```bash
==========================
basic-with-newline
==========================
hello

---

(source_file)
```

This passes because all whitespace including spaces and newlines are considered `extras` by default,
as can be verified from the `grammar.json` file.

```json
{
  "name": "baseline",
  "rules": {
    "source_file": {
      "type": "STRING",
      "value": "hello"
    }
  },
  "extras": [
    {
      "type": "PATTERN",
      "value": "\\s"
    }
  ],
  "conflicts": [],
  "precedences": [],
  "externals": [],
  "inline": [],
  "supertypes": []
}
```

To specify only spaces and not tabs or newlines as insignificant whitespace,
we have to explicitly add an `extras` key to our `grammar.js` file.

```javascript
module.exports = grammar({
  name: "baseline",
  rules: {
    source_file: $ => 'hello',
  },
  extras: ($) => [' '],
});
```

Now, as expected, newlines are treated as significant, and thus will raise errors.

```bash
  corpus:
    ✗ *basic-with-newline

1 failure:

expected:* / actual:|

  1. basic-with-newline:

    | (source_file
    |   (ERROR
    |     (UNEXPECTED '\n')))

    * (source_file)
```

To re-enable newlines through another route, just to flex our chops, we could modify our 
grammar as follows:

```javascript
module.exports = grammar({
  name: "baseline",
  rules: {
    source_file:  ($) => $._source_file,
    _source_file: ($) => seq(repeat('\n'), 'hello', repeat('\n')),
  },
  extras: ($) => [' '],
});
```

which then causes the test to pass:

```bash
  corpus:
    ✓ basic-with-newline
```

The generated `grammar.json`:

```json
{
  "name": "baseline",
  "rules": {
    "source_file": {
      "type": "SYMBOL",
      "name": "_source_file"
    },
    "_source_file": {
      "type": "SEQ",
      "members": [
        {
          "type": "REPEAT",
          "content": {
            "type": "STRING",
            "value": "\n"
          }
        },
        {
          "type": "STRING",
          "value": "hello"
        },
        {
          "type": "REPEAT",
          "content": {
            "type": "STRING",
            "value": "\n"
          }
        }
      ]
    }
  },
  "extras": [
    {
      "type": "STRING",
      "value": " "
    }
  ],
  "conflicts": [],
  "precedences": [],
  "externals": [],
  "inline": [],
  "supertypes": []
}
```

This demonstrates how there can be many ways of expressing the same grammar.


<hr/>

## newlines

Assume we have a log file that is divided by sections with lines starting with
`PASS` or `FAIL`. The lines following the 'header' belong to that section, and
we want to count/navigate those sections.

```bash
path: test_main.go
PASS: 
  😸 sdfsd sdfsdfsd 
    ssdfsdf
  😸 sdfsd sdfsdfsd 
    ssdfsdf
  😸 sdfsd sdfsdfsd 
    ssdfsdf
  😸 sdfsd sdfsdfsd 
    ssdfsdf
FAIL: 
  🐻 sdfsd sdfsdfsd 
    fsdfsdf
FAIL: 
  🐶 sdfsd sdfsdfsd 
    fsdfsdf
```

How do we write simple grammar to do that?

TODO: On a sidenote, the following causes `tree-sitter test` to hang, and rapidly consume RAM
(upto 13GB when I killed it).

```javascript
rules: {
  document: ($) => choice($.line),
  line:     ($) => seq(/[^\n]*/, repeat('\n')),
},
extras: ($) => [' '],
```

Changing the `*` to `+` in the pattern causes it to behave properly again.

```javascript
  line: ($) => seq(/[^\n]+/, repeat('\n')),
```

Finally, this works:

```javascript
  rules: {
    document: ($) => repeat1(choice($.pass, $.fail)),
    pass:     ($) => seq('PASS', repeat($._line)),
    fail:     ($) => seq('FAIL', repeat($._line)),
    _line:    ($) => seq(/[^\n]+/, repeat('\n')),
  },
  extras: ($) => [' '],
```

However, we see that all the sections get consumed by the first.

```bash
expected:* / actual:|

  1. *basic:

    | (document
    |   (pass))
    *  (pass)
    *  (fail)
    *  (fail))
```

This is easy to rectify, by adding a precedence to make `_line` the least
urgent rule (by default, precedence is 0).

```javascript
  rules: {
    document: ($) => repeat(choice($.pass, $.fail)),
    pass:     ($) => prec(1, seq(token('PASS:\n'), repeat($._line))),
    fail:     ($) => prec(1, seq(token('FAIL:\n'), repeat($._line))),
    _line:    ($) => seq(/[^\n]+/, optional('\n')),
  },
  extras: $ => [' '],
```

Finally, we get what we needed, where each 'section' is properly captured:

```bash
  corpus:
    ✓ *pass
    ✓ *basic
```

<hr/>
