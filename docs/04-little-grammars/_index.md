---
title: "Little grammars"
date: 2024-01-27T13:21:21+01:00
draft: true
---

Defining simple grammars to practice thinking about language syntax.

- [ ] valid-json
- [ ] taskwarrior
- [ ] arandr

After adding:

```javascript
contact: ($) => seq("{", commaSep($.user_pair), commaSep($.phone_pair), "}"),
user_pair: ($) =>
  seq(
    field("key", "user"),
    ":",
    field("value", $.string),
  ),
phone_pair: ($) =>
  seq(
    field("key", "phone"),
    ":",
    field("value", $.number),
  ),
```

And changing

```javascript
  choice($.object,   $.contact,   $.array, $.number, $.string, $.true, $.false, $.null),
```

We get:

```
Unresolved conflict for symbol sequence:

  '{'  '}'  •  '{'  …

Possible interpretations:

  1:  (contact  '{'  '}')  •  '{'  …
  2:  (object  '{'  '}')  •  '{'  …

Possible resolutions:

  1:  Specify a higher precedence in `object` than i
n the other rules.
  2:  Specify a higher precedence in `contact` than
in the other rules.
  3:  Add a conflict for these rules: `object`, `con
tact`

Generate failed:

[Process exited 1]
```

Finally, resolved with:

```javascript
object: ($) => prec(-1, seq("{", commaSep($.pair), "}")),
contact: ($) => prec(1, seq("{", $.user_pair, ",", $.phone_pair, "}")),
pair: ($) =>
  seq(
    field("key", choice($.string, $.number)),
    ":",
    field("value", $._value),
  ),
user_pair: ($) =>
  seq(
    '"user"',
    ":",
    field("value", $.string),
  ),
phone_pair: ($) =>
  seq(
    '"phone"',
    ":",
    field("value", $.number),
  ),
```

Now this works:

```
================================================================================
*not-contacts
================================================================================

[{"users": "abc", "phone": 13247}]

--------------------------------------------------------------------------------

(document (array (object
    (pair
      (string (string_content))
      (string (string_content)))
    (pair
      (string (string_content))
      (number))
)))


================================================================================
*contacts
================================================================================

[{"user": "abc", "phone": 13247}]

--------------------------------------------------------------------------------

(document (array (contact
    (user_pair (string (string_content)))
    (phone_pair (number))
)))
```

If we delete the general object, we get:

```javascript
_value: ($) =>
  choice($.contact, $.array, $.number, $.string, $.true, $.false, $.null), // No $.object
```

We have a parser that validates our data.

```
================================================================================
*only-contacts
================================================================================

[{"user": "abc", "phone": 13247}, {"user": "abc", "phone": "518-CALL-ME"}]

--------------------------------------------------------------------------------

(document (array (contact
      (user_pair (string (string_content)))
      (phone_pair (number)))
    (ERROR
      (user_pair (string (string_content)))
      (number) (UNEXPECTED 'C'))))
```
