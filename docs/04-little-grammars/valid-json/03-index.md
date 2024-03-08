---
title: "valid-json"
draft: true
date: 2024-01-01T08:00:00-06:00
---

In this grammar, we want to only allow json data of the following form:

```json
[{"user": "a", "phone": 563125}, {"user": "b", "phone": 632568}, {...}]
```

## baseline

To do this, we override the simple `grammar.js` from `tree-sitter-json`. The part
we are interested in is:

```javascript
  rules: {
    document: ($) => repeat($._value),
    _value:   ($) => choice($.contact, $.object, $.array, $.number, $.string, $.true, $.false, $.null),
    object:   ($) => prec(-1, seq("{", commaSep($.pair), "}")),
    pair:     ($) => seq(field("key", choice($.string, $.number)), ":", field("value", $._value),),
    ...
});
```

## valid-json

So what we need to do is override `$.object` to allow only contacts by adding new rules:

```javascript
contact:    ($) => seq("{", commaSep($.user_pair), commaSep($.phone_pair), "}"),
user_pair:  ($) => seq(field("key", "user"), ":", field("value", $.string)),
phone_pair: ($) => seq(field("key", "phone"), ":", field("value", $.number)),
```

And changing `_value` to include our `contact`,

```javascript
  choice($.object, $.contact, $.array, $.number, $.string, $.true, $.false, $.null),
```

However, we run into precedence issues:

```bash
Unresolved conflict for symbol sequence:

  '{'  '}'  •  '{'  …

Possible interpretations:

  1:  (contact  '{'  '}')  •  '{'  …
  2:  (object  '{'  '}')  •  '{'  …

Possible resolutions:

  1:  Specify a higher precedence in `object` than in the other rules.
  2:  Specify a higher precedence in `contact` than in the other rules.
  3:  Add a conflict for these rules: `object`, `contact`

Generate failed:

[Process exited 1]
```

So, all we need to do is add a precedence rule:

```javascript
object: ($) => prec(-1, seq("{", commaSep($.pair), "}")),
contact: ($) => seq("{", $.user_pair, ",", $.phone_pair, "}"),
```

Now this works:

```bash
================================================================================
not-contacts
================================================================================

[{"users": "abc", "phone": 13247}]

--------------------------------------------------------------------------------

(document (array 
  (object
    (pair
      (string (string_content))
      (string (string_content)))
    (pair
      (string (string_content))
      (number)))))

================================================================================
contacts
================================================================================

[{"user": "abc", "phone": 13247}]

--------------------------------------------------------------------------------

(document (array 
  (contact
    (user_pair (string (string_content)))
    (phone_pair (number)))))
```

If we delete the general `object`, we get:

```javascript
_value: ($) => choice($.contact, $.array, $.number, $.string, $.true, $.false, $.null), // No $.object
```

and we have a parser that validates our data.

```bash
================================================================================
*only-contacts
================================================================================

[{"user": "abc", "phone": 13247}, {"user": "abc", "phone": "518-CALL-ME"}]

--------------------------------------------------------------------------------

(document (array 
   (contact
      (user_pair (string (string_content)))
      (phone_pair (number)))
   (object
     (ERROR
       (user_pair (string (string_content)))
       (number) (UNEXPECTED 'C')))))
```

We can even clean up the resulting nodes to hide the useless ones:

```javascript
    contact:     ($) => seq("{", $._user_pair, ",", $._phone_pair, "}"),
    user:        ($) => alias($.string_content, "phone"),
    _user:       ($) => choice(seq('"', '"'), seq('"', $.user, '"')),
    phone:       ($) => alias($.number, "phone"),
    _user_pair:  ($) => seq('"user"', ":", field("user", $._user)),
    _phone_pair: ($) => seq('"phone"', ":", field("phone", $.phone)),
```

resulting in the output s-expression:

```bash
================================================================================
*contacts
================================================================================

[{"user": "abc", "phone": 13247}, {"user": "def", "phone": 98745}]

--------------------------------------------------------------------------------

(document (array
   (contact (user) (phone))
   (contact (user) (phone))))
```

<hr/>
