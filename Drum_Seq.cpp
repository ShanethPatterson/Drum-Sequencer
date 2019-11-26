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
#include <EEPROM.h>

//--------------------------------------------------------------------------------------------------------------------------
// Constants
#define MAXLENGTH \
    64  // only god knows what will actually happen if this variable changes.
        // Leaving it at 64 for now.
#define STARTINGVELOCITY 75
#define MIDISYNCENABLED 0

// Menu System
#define menuLBtn 0
#define menuRBtn 0
#define menuUpBtn 0
#define menuDnBtn 0
#define display 0

// Channel Selection
const int bankLeds[4] = {5, 4, 3, 2};
#define chUpBtn 31
#define chDnBtn 35

// Page Selection
#define pgLBtn 36
#define pgRBtn 34

// Controls
#define POT 33
#define SWITCH 32
#define TMPDN 37
#define TMPUP 30

// Matrix
#define matrixHeight 4
#define matrixWidth 8
#define selBlinkTime 200
#define selTrackBlueOffset 0
#define selTrackOffColor 0x101010

//--------------------------------------------------------------------------------------------------------------------------
// Global Variables
unsigned int step = 0;
bool run = false;
unsigned int tempo = 165;
unsigned int projectLength = 16;
unsigned int timeSig = 4;
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
        pitch = 36 + 30 - (id * 2);  // to follow GM drum mapping
        channel = 1;
    }
    int getNote(int n) { return abs(notes[n]); }
    int getNote() { return abs(notes[step]); }
    bool noteSelected(int n) { return (notes[n] < 0); }
    bool noteSelected() { return (notes[step] < 0); }
    void selectNote(int n) {
        if (notes[n] > 0) {
            notes[n] = 0 - notes[n];
        }
    }
    void toggleNoteSel(int n) { notes[n] = -notes[n]; }
    void toggleNoteOn(int n) {
        if (notes[n] == 0) {
            notes[n] = STARTINGVELOCITY;
        } else {
            notes[n] = 0;
        }
        writeNoteToEeprom(n);
    }
    void deselectAllNotes() {
        for (int i = 0; i < MAXLENGTH; i++) {
            notes[i] =
                abs(notes[i]);  // using sign flip to indicate selected
                                // was a good choice for this very reason.
        }
    }
    void setVelocity(int n, int vel) {
        notes[n] = vel;
        writeNoteToEeprom(n);
    }
    void writeNoteToEeprom(int n) {
        int addr = (id * MAXLENGTH) + n;
        Serial.printf("Writing value %d to EEPROM @addr %d\n", addr, n);
        EEPROM.write(addr, notes[n]);
    }
    void readTrackFromEeprom() {
        int addr = (id * MAXLENGTH);
        for (unsigned int i = 0; i < projectLength; i++) {
            notes[i] = EEPROM.read(addr + i);
            Serial.printf("Reading value %d from EEPROM @addr %d\n", addr + i,
                          notes[i]);
        }
    }
};
int8_t Track::numTracks = 0;
Track tracks[16] = Track();
void deselectAllNotesGlobal() {
    for (int i = 0; i < 16; i++) {
        tracks[i].deselectAllNotes();
    }
}

class ShaneButton {
   private:
    int pin;

   public:
    bool state;
    bool toggleState;
    ShaneButton() {}
    ShaneButton(int ipin) {
        pinMode(ipin, INPUT);
        pin = ipin;
        state = 0;
        toggleState = false;
    }
    bool stateChanged() {
        bool current = digitalRead(pin);
        if (current != state) {
            // state changed
            state = current;
            delay(10);
            return true;
        }
        return false;
    }
    bool pressed() {
        bool current = digitalRead(pin);
        if (current != state) {
            // state changed
            state = current;
            delay(10);
            return state;
        }
        return false;
    }
    bool updateState() {
        bool current = digitalRead(pin);
        if (current != state) {
            // state changed
            state = current;
            if (state == 1) {
                toggleState = !toggleState;
            }
            delay(10);
        }
        return state;
    }
    bool getToggleState() {
        updateState();
        return toggleState;
    }
};

//--------------------------------------------------------------------------------------------------------------------------
// Trellis

Adafruit_NeoTrellis trellisArray[matrixHeight / 4][matrixWidth / 4] = {
    {Adafruit_NeoTrellis(0x2E), Adafruit_NeoTrellis(0x2F)}};
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellisArray,
                              (matrixHeight / 4), (matrixWidth / 4));
TrellisCallback tkeyPressed(keyEvent e);
int8_t lastBlinkTime, page, bank, selTrack = 15, selNote, pageOffset;
const int8_t numPixels = matrixWidth * matrixHeight;
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
        selTrack = tracks[0].numTracks - 1;  // AHHHH ZERO INDEXING GOT ME AGAIN
    }
}
void pageLeft() {
    Serial.println("Left");
    if (step > matrixWidth) {
        step -= matrixWidth;
    } else {
        step = projectLength - 1;
    }
}
void pageRight() {
    if (step < (projectLength - matrixWidth)) {
        step += matrixWidth;
    } else {
        step = 0;
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

    page = (step) / matrixWidth;
    pageOffset = page * matrixWidth;

    for (int8_t y = 0; y < matrixHeight; y++) {
        for (int8_t x = 0; x < matrixWidth; x++) {
            // start by turning cell off
            trellis.setPixelColor(x, y, 0x000000);

            {  // drawing cell
                if (dispTracks[y].getNote(x + pageOffset) != 0) {
                    int8_t blueVal = 0;
                    if (selTrack % matrixHeight == y) {
                        blueVal = selTrackBlueOffset;  // defined wayyy up there
                                                       // in the constants
                    }
                    trellis.setPixelColor(
                        x, y,
                        seesaw_NeoPixel::Color(
                            dispTracks[y].getNote(x + pageOffset) * 2,
                            255 - dispTracks[y].getNote(x + pageOffset) * 2,
                            blueVal));  // velocity will bring it from green
                                        // up to red
                } else if (selTrack % matrixHeight == y) {
                    trellis.setPixelColor(x, y, selTrackOffColor);
                }
            }
            {  // selection stuff
                if (dispTracks[y].noteSelected(x + pageOffset)) {
                    if ((millis() / selBlinkTime) % 2 == 0) {
                        // turn cell off to blink
                        trellis.setPixelColor(x, y, 0x000000);
                    }
                }
            }  // end selection stuff
            // finally, if the project is running and it is the current step,
            // light up
            if (run && (step == x + pageOffset)) {
                trellis.setPixelColor(x, y, 0xFFFFFF);
            }
        }
    }
    trellis.show();
}
TrellisCallback tkeyPressed(keyEvent e) {
    int x = (e.bit.NUM % matrixWidth) + pageOffset;
    int y = e.bit.NUM / matrixWidth;
    Serial.printf("E: %d, (%d, %d)\n", e.bit.NUM, x, y);
    // i really hope this works
    deselectAllNotesGlobal();

    selTrack = (bank * matrixHeight) + y;
    selNote = x;
    tracks[selTrack].toggleNoteOn(x);
    tracks[selTrack].selectNote(x);
    usbMIDI.sendNoteOn(tracks[selTrack].pitch, 80, tracks[selTrack].channel);
    return 0;
}
void setupMatrix() {
    for (int y = 0; y < matrixHeight; y++) {
        for (int x = 0; x < matrixWidth; x++) {
            Serial.printf("%d %d", x, y);
            trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
            // trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
            trellis.registerCallback(x, y, tkeyPressed);
        }
    }
}
void processMatrix() {
    drawMatrix();
    trellis.read();
}
// end trellis/matrix
//--------------------------------------------------------------------------------------------------------------------------
// Controls
unsigned int lastPotVal = 0;
bool lastSwitchVal = 0;
bool potVelActive = true;  // change if the pot is being used for something else
                           // so it doesn't change velocities
ShaneButton chUp = ShaneButton(chUpBtn);
ShaneButton chDn = ShaneButton(chDnBtn);
ShaneButton pgL = ShaneButton(pgLBtn);
ShaneButton pgR = ShaneButton(pgRBtn);
ShaneButton tempoDown = ShaneButton(TMPDN);
ShaneButton tempoUp = ShaneButton(TMPUP);

void controls() {
    // pot velocity adjustment
    if (potVelActive) {
        if (analogRead(POT) != lastPotVal) {
            if (tracks[selTrack].noteSelected(selNote)) {
                tracks[selTrack].setVelocity(
                    selNote, 0 - map(analogRead(POT), 0, 1027, 1, 127));
                lastPotVal = analogRead(POT);
            }
        }
    }
    // Switch to stop and start seq
    if (digitalRead(SWITCH)) {
        if (lastSwitchVal == 0) {
            deselectAllNotesGlobal();
            lastSwitchVal = 1;
            step = 0;
        }
        run = true;
    } else {
        run = false;
    }
    // LED to show bank
    for (int i = 0; i < 4; i++) {
        if (i == bank) {
            digitalWrite(bankLeds[i], HIGH);
        } else {
            digitalWrite(bankLeds[i], LOW);
        }
    }
    // The following code controls the matrix, so the functions below can be
    // found in the matrix section
    // Bank Changing
    if (chUp.pressed()) {
        bankDown();  // these are purposefully flipped
    }
    if (chDn.pressed()) {
        bankUp();  // these are purposefully flipped
    }
    // Page Changing
    if (pgL.pressed()) {
        pageLeft();
    }
    if (pgR.pressed()) {
        pageRight();
    }
    // tempo changing
    if (tempoDown.pressed()) {
        tempo--;
    }
    if (tempoUp.pressed()) {
        tempo++;
    }
}
//--------------------------------------------------------------------------------------------------------------------------
// Sequencer
int midiClockMod = 6;
int midiClockStep = 0;
bool midiClockActive = false;
unsigned long lastStepTime = 0;
void beforeStep() {
    // kill previous notes
    for (int i = 0; i < 16; i++) {
        usbMIDI.sendNoteOff(tracks[i].pitch, 0, tracks[i].channel);
    }
}
void afterStep() {
    // send midi for new notes
    for (int i = 0; i < 16; i++) {
        if (tracks[i].getNote()) {
            usbMIDI.sendNoteOn(tracks[i].pitch, tracks[i].getNote(),
                               tracks[i].channel);
        }
    }
}
// MIDI CLOCKING
void RealTimeSystem(byte realtimebyte) {
    if (realtimebyte == 248 && run) {  // midi clock tick
        midiClockActive = true;

        midiClockStep++;
        Serial.println(midiClockStep);
        if (midiClockStep >= midiClockMod) {
            beforeStep();
            midiClockStep = 0;

            if ((step + 1) < projectLength) {
                step++;
            } else {
                step = 0;
            }
            afterStep();
        }
    }
    if (realtimebyte = 250) {  // midi for start
                               //  midiClockStep = 0;
    }
    if (realtimebyte == 252) {  // midi for stop
        midiClockActive = false;
        midiClockStep = 0;
    }
}
void seq() {
    // Self Clock
    // first, convert tempo (in bpm) to millis between steps
    int millisBetweenSteps = (60000 / tempo);
    if (millis() > lastStepTime + millisBetweenSteps && !midiClockActive) {
        lastStepTime = millis();
        beforeStep();
        if ((step + 1) < projectLength) {
            step++;
        } else {
            step = 0;
        }
        afterStep();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
// Setup

void setup() {
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    Serial.print("Start... ");
    digitalWrite(13, HIGH);
    delay(1000);
    Serial.println("Serial OK!");

    if (!trellis.begin()) {
        Serial.println("Critical: Matrix Failed to Initialize!!");
        // while (true) {
        //     digitalWrite(13, LOW);
        //     delay(100);
        //     digitalWrite(13, HIGH);
        //     delay(100);
        // }
    }
    // make sure trellis is working properly by displaying a little
    // animatino

    for (int i = 0; i < numPixels; i++) {
        trellis.setPixelColor(i, 0x000000);
        trellis.show();
    }
    for (int i = 0; i < numPixels; i++) {
        digitalWrite(13, LOW);
        trellis.setPixelColor(i, 0x100035);
        trellis.show();
        delay(25);
    }

    // Load from EEPROM prompt
    bool waitFlag = true;
    int Lchar[7] = {0, 8, 16, 24, 25, 26, 27};
    for (int i = 0; i < 7; i++) {
        trellis.setPixelColor(Lchar[i], 0xFFFFFF);
    }

    trellis.show();

    while (waitFlag) {
        if (chDn.pressed() || pgL.pressed()) {
            Serial.println("Clearing EEPROM. Starting from scratch.");
            for (int i = 0; i < EEPROM.length(); i++) {
                EEPROM.write(i, 0);
            }
            waitFlag = false;
        }
        if (chUp.pressed() || pgR.pressed()) {
            Serial.println("Reading from EEPROM.");
            for (int i = 0; i < 16; i++) {
                tracks[i].readTrackFromEeprom();
            }
            waitFlag = false;
        }
    }

    // Create callbacks
    setupMatrix();

    // initialize controls
    for (int i = 0; i < 4; i++) {
        pinMode(bankLeds[i], OUTPUT);
    }
    pinMode(SWITCH, INPUT);

    //
    if (MIDISYNCENABLED) {
        usbMIDI.setHandleRealTimeSystem(RealTimeSystem);
    }
    Serial.println("\nStarting loop");
    for (int i = 0; i < numPixels; i++) {
        trellis.setPixelColor(i, 0x000000);
        trellis.show();
        delay(25);
    }
}

//--------------------------------------------------------------------------------------------------------------------------
// Loop
void loop() {
    processMatrix();
    controls();
    if (run) {
        seq();
    }
    if (MIDISYNCENABLED) {
        usbMIDI.read();
    }
}
