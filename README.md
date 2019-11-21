# Drum Seq

## A feature rich arduino drum sequencer running on a Teensy 3.5

Still in planning stages. Tentatively, the drum sequencer will act strictly as a midi controller (although an internal audio mode is not off the table completely).
The sequencer is controlled by a 4 tall x 8 wide matrix of RGB Leds with buttons reffered to as a NeoTrellis.
There will be 16 "tracks" which can broadcast any midi note on any midi channel over USBMidi.

### WORKING:

-8 steps
-all 16 tracks with the two buttons to bank up or down (with bounds checking of course)
-Velocity control
-Note toggling and selection
-MIDI output

### Todo:


Project Planning:
https://l.shaneth.com/drumseqplan

Shane Patterson
shaneth.com

---

Project Length: 8-64 steps (1/2 a measure to 4 measures of 16th notes at 4/4. In 6/8 that would give us 5.33 measures but I'll cut it off at 4 and lose the last page of steps)
