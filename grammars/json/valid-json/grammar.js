module.exports = grammar({
  name: "json",
  extras: ($) => [/\s/, $.comment],
  supertypes: ($) => [$._value],
  rules: {
    document: ($) => repeat($._value),
    _value: ($) =>
      choice($.contact, $.object, $.array, $.number, $.string, $.true, $.false, $.null),
    object: ($) => prec(-1, seq("{", commaSep($.pair), "}")),
    pair: ($) => seq(field("key", choice($.string, $.number)), ":", field("value", $._value),),
    contact: ($) => seq("{", $._user_pair, ",", $._phone_pair, "}"),
    user: ($) => alias($.string_content, "phone"),
    _user: ($) => choice(seq('"', '"'), seq('"', $.user, '"')),
    phone: ($) => alias($.number, "phone"),
    _user_pair: ($) => seq('"user"', ":", field("user", $._user)),
    _phone_pair: ($) => seq('"phone"', ":", field("phone", $.phone)),
    array: ($) => seq("[", commaSep($._value), "]"),
    string: ($) => choice(seq('"', '"'), seq('"', $.string_content, '"')),
    string_content: ($) =>
      repeat1(choice(token.immediate(prec(1, /[^\\"\n]+/)), $.escape_sequence)),
    escape_sequence: (_) =>
      token.immediate(seq("\\", /(\"|\\|\/|b|f|n|r|t|u)/)),
    number: (_) => {
      const decimal_digits = /\d+/;
      const signed_integer = seq(optional("-"), decimal_digits);
      const exponent_part = seq(choice("e", "E"), signed_integer);
      const decimal_integer_literal = seq(
        optional("-"),
        choice("0", seq(/[1-9]/, optional(decimal_digits))),
      );
      const decimal_literal = choice(
        seq(
          decimal_integer_literal,
          ".",
          optional(decimal_digits),
          optional(exponent_part),
        ),
        seq(decimal_integer_literal, optional(exponent_part)),
      );
      return token(decimal_literal);
    },
    true: (_) => "true",
    false: (_) => "false",
    null: (_) => "null",
    comment: (_) =>
      token(
        choice(seq("//", /.*/), seq("/*", /[^*]*\*+([^/*][^*]*\*+)*/, "/")),
      ),
  },
});

// Creates a rule to match one or more of the rules separated by a comma
function commaSep1(rule) {
  return seq(rule, repeat(seq(",", rule)));
}

// Creates a rule to optionally match one or more of the rules separated by a comma
function commaSep(rule) {
  return optional(commaSep1(rule));
}
