const RED = "[31m";
const REDSEL = "[31m* ";
const GREEN = "[32m";
const GREENSEL = "[32m* ";
const RESET = "[0m";
const PASS = "✓";
const FAIL = "✗";
const FAILURE = "failure:";
const EXP_ACT = "[32mexpected[0m / [31mactual[0m";

module.exports = grammar({
  name: "baseline",
  rules: {
    source_file: ($) =>
      seq(
        repeat(choice($.pass, $.fail)), // header
        repeat(seq($.fail_head, EXP_ACT)),
        repeat(seq($.fail_title)),
      ),
    _identifier: ($) =>
      /[_@a-zA-Z0-9()\[\],.\xC0-\xD6\xD8-\xDE\xDF-\xF6\xF8-\xFF-]+/,
    _number: ($) => /[_0-9]+/,
    num_fails: ($) => $._number,
    idx: ($) => seq($._number, "."),
    name: ($) => $._identifier,
    pass: ($) => seq(PASS, seq(choice(GREEN, GREENSEL), $.name, RESET)),
    fail: ($) => seq(FAIL, seq(choice(RED, REDSEL), $.name, RESET)),
    fail_head: ($) => seq($.num_fails, FAILURE),
    fail_title: ($) => seq($.idx, repeat("*"), seq($.name, "::")),
    // fail_body: ($) => repeat($.lines),
    // lines: ($) => repeat(seq(repeat($._identifier), "\n")),
  },
});
