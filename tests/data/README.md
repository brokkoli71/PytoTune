# Test Data

This directory contains super simple audio files, that are required for the unit tests.
This files should be minimal in size.

## Naming

Here one can find teh naming conventions for the more complicated names. In General it has the following format:

`<type_of_sound>(_<attribute_name><attribute_value>?)*.wav`

### Types of sound

- `sin` basic sine wave
- `saw` basic saw wave
- `square` basic square wave
- `<sound>-raise` basic continuous raise played by `<sound>`, starting at `fstart` until `fend`
- `piano` one (synthesized) piano tone
- `strings` one (synthesized) strings tone
- `voice` one (synthesized) void tone
- `<sound>-majorscale` A major scale played by `<sound>`, starting at `fstart` until `fend`

### Possible Attributes

- `f` frequency of the sound
    - `fstart` frequency of the start of the sound, e.g. in a raise
    - `fend` frequency of the end of the sound, e.g. in a raise
- `i` intensity in percent, e.g `i80` sets the maximum pressure level to be `0.8`
- `sr` sample rate in Hz
- `af` audio format of the wave file. Only `af1` and `af3` are supported.
- `cd` content duration. This is the duration of the audio excluding the tail
- `tail` a flag to signal that some (reverb) tail is included
- `pause` a flag to signal that a pause is included

### Examples

- `sin_f440_i80_sr44100_af1.wav` is pure sine wave at `440Hz` with maximal pressure level `0.8`. The sample rate is
  `44100Hz` and the audio format is `1`
- `voice-majorscale_fstart220_fend440_cd6_tail_pause` is a synthesized voiced singing a major scale from A3 (220Hz) to
  A4 (440Hz) with some pause in between. This takes in 6s and a reverb tail is included.