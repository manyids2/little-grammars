---
title: "xrandr"
draft: true
date: 2024-01-01T08:00:00-010:00
---

Explanation pending.

Grammar to parse output of xrandr:

```bash
================================================================================
full
================================================================================

Screen 0: minimum 8 x 8, current 2560 x 1440, maximum 32767 x 32767
DP-0 disconnected primary (normal left inverted right x axis y axis)
DP-1 disconnected (normal left inverted right x axis y axis)
HDMI-0 connected 2560x1440+0+0 (normal left inverted right x axis y axis) 597mm x 336mm
   2560x1440     59.95*+
   2048x1080     60.00  
   1920x1200     59.88  
   1920x1080     60.00    59.94    50.00  
DP-4 disconnected (normal left inverted right x axis y axis)

--------------------------------------------------------------------------------

(document
  (screen (number)
    (resolution (xres) (yres))
    (resolution (xres) (yres))
    (resolution (xres) (yres)))
  (disconnected (display) (primary) (props))
  (disconnected (display) (props))
  (connected (display)
    (geometry (xres) (yres) (xoff) (yoff))
    (props) 
    (resolution_with_units (xres) (yres))
    (resolutions
      (resolution (xres) (yres)) (number) (mark) (mark)
      (resolution (xres) (yres)) (number)
      (resolution (xres) (yres)) (number)
      (resolution (xres) (yres)) (number) (number) (number)))
  (disconnected (display) (props))
```

<hr/>

The entire `grammar.js`:

```javascript
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
```

<hr/>
