module.exports = grammar({
  name: "baseline",
  rules: {
    source_file: $ => $._source_file,
    _source_file: $ => seq(repeat('\n'), 'hello', repeat('\n')),
  },
  extras: ($) => [' '],
});
