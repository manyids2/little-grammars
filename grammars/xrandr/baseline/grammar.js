module.exports = grammar({
  name: "xrandr",
  rules: {
    // document: ($) => repeat(choice($.screen, $.disconnected, $.connected)),
    document: ($) => $.resolutions,
    number: (_) => prec.left(-2, /[\d\.]+/),
    delim: (_) => /\n/,
    resolution: ($) => prec.left(-1, seq($.number, 'x', $.number)),
    geometry: ($) => seq($.resolution, '+', $.number, '+', $.number),
    display: ($) => /[a-zA-Z0-9-]+-[0-9]+/,
    // Screen
    screen: ($) => seq(
      seq('Screen', field("screen_number", $.number)), ':',
      'minimum', field('minimum', $.resolution), ',',
      'current', field('current', $.resolution), ',',
      'maximum', field('maximum', $.resolution),
    ),
    // Disconnected
    props: ($) => seq('(', repeat(choice('normal', 'left', 'right', 'x', 'y', 'axis', 'inverted')), ')'),
    primary: ($) => 'primary',
    disconnected: ($) => seq(
      field('name', $.display), 'disconnected', repeat(field('primary', $.primary)),
      $.props,
    ),
    // Connected
    connected: ($) => seq(
      field('name', $.display), 'connected', repeat(field('primary', 'primary')), $.geometry,
      $.props,
      $.resolution_with_units,
      // $.resolutions, // BUG: Why is this throwing error only in the context of connected?
    ),
    number_with_units: ($) => seq($.number, /[a-zA-Z]+/),
    resolution_with_units: ($) => seq($.number_with_units, 'x', $.number_with_units),
    mark: ($) => choice('+', '*'),
    resolution_line: ($) => prec.left(1, seq($.resolution, repeat1(choice($.number, $.mark)))),
    resolutions: ($) => repeat1($.resolution_line),
  },
});
