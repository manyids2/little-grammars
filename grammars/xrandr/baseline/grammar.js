const num_with_units = (res) => seq(res, field("units", token.immediate(/[a-zA-Z]+/)));

module.exports = grammar({
  name: "xrandr",
  rules: {
    document: ($) => repeat(choice($.screen, $.disconnected, $.connected)),
    number: (_) => /[\d\.]+/,
    xres: ($) => alias($.number, "xres"),
    yres: ($) => alias($.number, "yres"),
    resolution: ($) => seq($.xres, 'x', $.yres),
    // Screen
    screen: ($) => seq(
      seq('Screen', field("screen_number", $.number)), ':',
      'minimum', field('minimum', $.resolution), ',',
      'current', field('current', $.resolution), ',',
      'maximum', field('maximum', $.resolution),
    ),
    display: (_) => /[a-zA-Z0-9-]+-[0-9]+/,
    // Disconnected
    props: (_) => seq('(', repeat1(choice('normal', 'left', 'right', 'x', 'y', 'axis', 'inverted')), ')'),
    primary: (_) => 'primary',
    disconnected: ($) => seq(
      field('display', $.display), 'disconnected', repeat(field('primary', $.primary)),
      $.props,
    ),
    // Connected
    xoff: ($) => alias($.number, "xoff"),
    yoff: ($) => alias($.number, "yoff"),
    geometry: ($) => seq(
      $.xres, token.immediate('x'),
      $.yres, token.immediate('+'),
      $.xoff, token.immediate('+'),
      $.yoff
    ),
    resolution_with_units: ($) => seq(num_with_units($.xres), 'x', num_with_units($.yres)),
    mark: (_) => choice(token.immediate('+'), token.immediate('*')),
    // -- may not always be terminated by \n?
    _resolution_line: ($) => seq($.resolution, repeat1(choice($.number, $.mark)), '\n'),
    resolutions: ($) => repeat1($._resolution_line),
    connected: ($) => seq(
      field('display', $.display), 'connected', repeat(field('primary', $.primary)), $.geometry,
      $.props,
      $.resolution_with_units,
      $.resolutions,
    ),
  },
});
