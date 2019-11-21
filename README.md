# Drum Seq

## A feature rich arduino drum sequencer running on a Teensy 3.5

Still in planning stages. Tentatively, the drum sequencer will act strictly as a midi controller (although an internal audio mode is not off the table completely).
The sequencer is controlled by a 4 tall x 8 wide matrix of RGB Leds with buttons reffered to as a NeoTrellis.
There will be 16 "tracks" which can broadcast any midi note on any midi channel over USBMidi.

### WORKING:

-16 steps
-all 16 tracks with the two buttons to bank up or down (with bounds checking of course)
-Velocity control
-Note toggling and selection
-MIDI output
-Page change buttons
-Run/Stop Switch
-GM Mapping
-Weird upside-down y axis hacks since I didn't think that NeoPixel counts row 0 as the top row, but I want the lowest midi notes on the bottom.

### Todo:

Make Midi DAW Sync Work.
Invent some way of changing project length, tempo, and time signature. Consider adding buttons and doing something fancy with the trellis array.
For project length, could just light up a number of the trellis pixels where 1 pixel/step giving us 1-32 steps. Time signature could be represented the same way. Tempo may require some other control. +/- buttons?

Shane Patterson
shaneth.com

---

Project Length: 8-64 steps (1/2 a measure to 4 measures of 16th notes at 4/4. In 6/8 that would give us 5.33 measures but I'll cut it off at 4 and lose the last page of steps)
