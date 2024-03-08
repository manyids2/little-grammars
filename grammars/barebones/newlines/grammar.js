module.exports = grammar({
  name: "baseline",
  rules: {
    document: $ => repeat(choice($.pass, $.fail)),
    pass: $ => prec(1, seq(token('PASS:\n'), repeat($._line))),
    fail: $ => prec(1, seq(token('FAIL:\n'), repeat($._line))),
    _line: $ => seq(/[^\n]+/, optional('\n')),
  },
  extras: $ => [' '],
});
