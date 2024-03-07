module.exports = grammar({
  name: "xrandr",
  conflicts: ($) => [[$._resolution_line]],
  rules: {
    document: ($) => repeat(choice($.screen, $.disconnected, $.connected)),
    number: ($) => /[\d\.]+/,
    xres: ($) => /[\d\.]+/,
    yres: ($) => /[\d\.]+/,
    resolution: ($) => prec.left(1, seq($.xres, 'x', $.yres)),
    geometry: ($) => seq(
      $.number, token.immediate('x'),
      $.number, token.immediate('+'),
      $.number, token.immediate('+'),
      $.number
    ),
    display: ($) => /[a-zA-Z0-9-]+-[0-9]+/,
    // Screen
    screen: ($) => seq(
      seq('Screen', field("screen_number", $.number)), ':',
      'minimum', field('minimum', $.resolution), ',',
      'current', field('current', $.resolution), ',',
      'maximum', field('maximum', $.resolution),
    ),
    // Disconnected
    props: ($) => seq('(', repeat1(choice('normal', 'left', 'right', 'x', 'y', 'axis', 'inverted')), ')'),
    primary: ($) => 'primary',
    disconnected: ($) => seq(
      field('name', $.display), 'disconnected', repeat(field('primary', $.primary)),
      $.props,
    ),
    // Connected
    _number_with_units: ($) => seq($.number, field("units", token.immediate(/[a-zA-Z]+/))),
    resolution_with_units: ($) => seq($._number_with_units, 'x', $._number_with_units),
    mark: ($) => choice(token.immediate('+'), token.immediate('*')),
    _resolution_line: ($) => seq($.resolution, repeat1(choice($.number, $.mark))),
    resolutions: ($) => repeat1($._resolution_line),
    connected: ($) => seq(
      field('name', $.display), 'connected', repeat(field('primary', $.primary)), $.geometry,
      $.props,
      $.resolution_with_units,
      optional($.resolutions),
    ),
  },
});
