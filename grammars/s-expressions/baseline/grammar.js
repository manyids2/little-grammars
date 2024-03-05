const PREC = {};
const PARENS_LEFT = "(";
const PARENS_RIGHT = ")";
const delim = (open, x, close) => seq(open, x, close);

module.exports = grammar({
  name: "sexp",
  rules: {
    sexp: ($) => $._sexp,
    _sexp: ($) => choice($.atom, $.list),
    atom: ($) => /[_@a-zA-Z0-9\xC0-\xD6\xD8-\xDE\xDF-\xF6\xF8-\xFF:-]+/,
    list: ($) => delim(PARENS_LEFT, repeat($._sexp), PARENS_RIGHT),
  },
});