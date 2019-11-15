// Drum Sequencer
// A feature-rich multichannel drum sequencer!
// Author 		Shane Patterson
// Date			11/7/19 1:22 PM
// Version		<#version#>
//  Created by Shane Patterson on 11/7/19.
//  Copyright © 2019 Shane Patterson. All rights reserved.
// See README for references

//--------------------------------------------------------------------------------------------------------------------------
// Libraries
#include <Adafruit_NeoTrellis.h>
//#include <Arduino.h>
//#include <algorithm>
//#include <vector>

//--------------------------------------------------------------------------------------------------------------------------
// Enviornment Constants
#define MAXLENGTH \
    64  // only god knows what will actually happen if this variable changes.
        // Leaving it at 64 for now.
#define STARTINGVELOCITY 75

// Menu System
#define menuLBtn 0
#define menuRBtn 0
#define menuUpBtn 0
#define menuDnBtn 0
#define display 0

// Channel Selection
const int chLeds[4] = {33, 34, 35, 36};
#define chUpBtn 0
#define chDnBtn 0

// Matrix
#define matrixHeight 4
#define matrixWidth 8
#define selBlinkTime 200
#define selTrackBlueOffset 127
#define selTrackOffColor 0x0000AA

//--------------------------------------------------------------------------------------------------------------------------
// Global Variables
int step = 0;

//--------------------------------------------------------------------------------------------------------------------------
// Classes
class Track {
   public:
    // these are all int8_t to minimize memory usage as I will be creating a lot
    // of these.
    static int8_t numTracks;
    int8_t id, pitch, channel;
    int8_t notes[MAXLENGTH];  // note values are normally 0-127. however,
                              // instead of creating a note class so that I
                              // could also store a selected boolean, I'm just
                              // going to flip the sign
    Track() {
        id = numTracks;
        numTracks++;  // static values ftw for ID'ing all these tracks and
                      // setting pitches automatically
        pitch = 36 + 16 - id;  // to follow GM mapping chromatically
        channel = 1;
    }
    int getNote(int n) { return abs(notes[n]); }
    int getNote() { return abs(notes[step]); }
    bool noteSelected(int n) { return (notes[n] < 0); }
    bool noteSelected() { return (notes[step] < 0); }
    void selectNote(int n) {
        if (notes[n] > 0) {
            notes[n] = -notes[n];
        }
    }
    void toggleNoteSel(int n) { notes[n] = -notes[n]; }
    void toggleNoteOn(int n) {
        if (notes[n] == 0) {
            notes[n] = STARTINGVELOCITY;
        }
    }
    void deselectAllNotes() {
        for (int i = 0; i < MAXLENGTH; i++) {
            notes[i] =
                abs(notes[i]);  // using sign flip to indicate selected
                                // was a good choice for this very reason.
        }
    }
};
int8_t Track::numTracks = 0;
Track tracks[16] = Track();

//--------------------------------------------------------------------------------------------------------------------------
// Trellis

Adafruit_NeoTrellis trellisArray[matrixHeight / 4][matrixWidth / 4] = {
    {Adafruit_NeoTrellis(0x2E), Adafruit_NeoTrellis(0x2F)}};
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellisArray,
                              (matrixWidth / 4), (matrixHeight / 4));
TrellisCallback tkeyPressed(keyEvent e);
int8_t lastBlinkTime, page, bank, selTrack;
const int8_t numPixels = matrixWidth * matrixHeight;
void setupMatrix() {
    for (int8_t y = 0; y < matrixHeight; y++) {
        for (int8_t x = 0; x < matrixWidth; x++) {
            trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
            trellis.registerCallback(x, y, tkeyPressed);
        }
    }
}
void bankUp() {
    if (bank !=
        matrixHeight - 1) {  //-1 because 0 indexing destroys my brain and life
        selTrack += matrixHeight;
    } else {
        selTrack = 0;
    }
}
void bankDown() {
    if (bank != 0) {
        selTrack -= matrixHeight;
    } else {
        selTrack = tracks[0].numTracks;
    }
}
void drawMatrix() {
    // figure out bank before page, since loading 16 track objects is
    // significantly slower than loading 4 for the future operations
    bank = selTrack / matrixHeight;
    int8_t bankOffset = bank * matrixHeight;
    Track dispTracks[matrixHeight] = {
        tracks[bankOffset], tracks[bankOffset + 1], tracks[bankOffset + 2],
        tracks[bankOffset + 3]};  // TODO: change to using pointers so that
                                  // future operations are super quick

    // figure out page
    if (step > matrixWidth) {
        page = step / matrixWidth;
    }

    for (int8_t y = bank * matrixHeight;
         y < (bank * matrixHeight) + matrixHeight; y++) {
        for (int8_t x = page * matrixWidth;
             x < (page * matrixWidth) + matrixWidth; x++) {
            {  // selection stuff
                if (dispTracks[y].noteSelected(x)) {
                    if ((millis() / selBlinkTime) % 2 == 0) {
                        // turn cell off to blink
                        trellis.setPixelColor(x, y, 0x000000);
                        break;  // we're done... for now
                        // if this doesn't work (I don't have all my parts
                        // while writing this and it is 4 am so I'm not sure
                        // what I'm doing anyways), use x++ instead to
                        // effetively break this loop but cause some funky
                        // things to happen with consecutive selected notes
                        // or use a flag for when the note is off
                    }
                }
            }  // end selection stuff
            {  // actually draw the cell otherwise
                if (dispTracks[y].getNote(x) != 0) {
                    int8_t blueVal = 0;
                    if (selTrack % matrixHeight == y) {
                        blueVal = selTrackBlueOffset;  // defined wayyy up there
                                                       // in the constants
                    }
                    trellis.setPixelColor(
                        x, y,
                        seesaw_NeoPixel::Color(
                            dispTracks[y].getNote(x) * 2,
                            255 - dispTracks[y].getNote(x) * 2,
                            blueVal));  // velocity will bring it from green
                                        // up to red
                } else if (selTrack % matrixHeight == y) {
                    trellis.setPixelColor(x, y, selTrackOffColor);
                }
            }
        }
    }
}
TrellisCallback tkeyPressed(keyEvent e) {
    int x = e.bit.NUM % matrixHeight;
    int y = e.bit.NUM / matrixHeight;
    // i really hope this works
    selTrack = (bank * matrixHeight) + y;
    tracks[selTrack].toggleNoteOn((page * matrixWidth) + x);
    return 0;
}
void processMatrix() { drawMatrix(); }
// end trellis/matrix

//--------------------------------------------------------------------------------------------------------------------------
// Setup
void setup() {
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    digitalWrite(13, HIGH);
    delay(500);
    Serial.println("Serial OK!");
    // setupMatrix();
    if (!trellis.begin()) {
        Serial.println("Critical: Matrix Failed to Initialize!!");
    }
    // make sure trellis is working properly
    for (int i = 0; i < numPixels; i++) {
        trellis.setPixelColor(i, 0x000000);
        trellis.show();
    }
    for (int i = 0; i < numPixels; i++) {
        digitalWrite(13, LOW);
        trellis.setPixelColor(i, 0xAA00FF);
        trellis.show();
        delay(75);
    }
    int okmsg[20] = {0,  1,  2,  3,  6,  7,  8,  11, 12, 13,
                     16, 19, 20, 21, 24, 25, 26, 27, 30, 31};
    for (int i = 0; i < 20; i++) {
        trellis.setPixelColor(okmsg[i], 0xFFFFFF);
        trellis.show();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
// Loop
void loop() {}
