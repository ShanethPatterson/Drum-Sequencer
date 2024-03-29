# Drum Seq

## A feature rich arduino drum sequencer running on a Teensy 3.5

Still in planning stages. Tentatively, the drum sequencer will act strictly as a midi controller (although an internal audio mode is not off the table completely).
The sequencer is controlled by a 4 tall x 8 wide matrix of RGB Leds with buttons reffered to as a NeoTrellis.
There are 16 "tracks" which can broadcast any midi note on any midi channel over USBMidi.

### WORKING:

- [x] 16 steps
- [x] all 16 tracks with the two buttons to bank up or down (with bounds checking of course)
- [x] Velocity control
- [x] Note toggling and selection
- [x] MIDI output
- [x] Page change buttons
- [x] Run/Stop Switch
- [x] GM Mapping
- [x] EEPROM that _automatically_ saves changes _as they are made_.
- [x] Option to load from EEPROM at startup. See TODO->
- [x] Weird upside-down y axis hacks since I didn't think that NeoPixel counts row 0 as the top row, but I want the lowest midi notes on the bottom.
- [x] Added tempo control with EEPROM recall on load.
- [x] Project length change.

### Todo:

- [ ] EEPROM clear confirmation: make whole array flash red and user must press left/down again to clear EEPROM, otherwise cancel clear and load.
  - [ ] Make Midi DAW Sync Work. Some code is in place, however it is not functional yet. It is possible all the midi clock messages from the DAW are overloading the teensy and without writing a scheduling algorithm, will always cause erratic behavior.
- [ ] Audio mode! There is some code in place to create audio, however it is not functional yet.

**Shane Patterson**
[shaneth.com](http://shaneth.com)

###EEPROM MAPPING:

- Address 0-960: Note velocites
- Address 4095: Tempo
- Address 4094: Length
