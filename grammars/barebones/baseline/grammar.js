module.exports = grammar({
  name: "baseline",
  rules: {
    source_file: ($) => seq(
      repeat($.metadata), // optional metadata
      $.corpus, // let it be public
      seq(repeat(choice($.pass, $.fail))) // tests
    ),
    _identifier: ($) => /[_@a-zA-Z0-9\xC0-\xD6\xD8-\xDE\xDF-\xF6\xF8-\xFF:-]+/,
    metadata: ($) => seq($._identifier, ":", $._identifier),
    corpus: ($) => seq("corpus", ":"),
    pass: ($) => seq("✓", repeat("*"), $._identifier),
    fail: ($) => seq("✗", repeat("*"), $._identifier),
  },
});
