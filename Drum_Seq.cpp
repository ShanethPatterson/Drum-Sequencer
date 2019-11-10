// Drum Sequencer
// A feature-rich multichannel drum sequencer!
// Author 		Shane Patterson
// Date			11/7/19 1:22 PM
// Version		<#version#>
//  Created by Shane Patterson on 11/7/19.
//  Copyright © 2019 Shane Patterson. All rights reserved.
// See README for references

//--------------------------------------------------------------------------------------------------------------------------
//Libraries
#include <Arduino.h>
#include <Adafruit_NeoTrellis.h>

//--------------------------------------------------------------------------------------------------------------------------
//Enviornment Constants

//Menu System
#define menuLBtn 0
#define menuRBtn 0
#define menuUpBtn 0
#define menuDnBtn 0
#define display 0

//Channel Selection
const int chLeds[4] = {33, 34, 35, 36};
#define chUpBtn 0
#define chDnBtn 0

//Matrix
#define matrixHeight 4
#define matrixWidth 8

//--------------------------------------------------------------------------------------------------------------------------
//Classes
class Track

class Matrix
{
private:
    static int width, height;
    Adafruit_NeoTrellis trellisArray[matrixHeight / 4][matrixWidth / 4] = {{Adafruit_NeoTrellis(0x2E), Adafruit_NeoTrellis(0x2F)}};
public:
    int numPixels = width * height;
    static Adafruit_MultiTrellis trellis;
    Matrix() {
        width=matrixWidth;
        height=matrixHeight;
        Adafruit_NeoTrellis trellisArray[height / 4][width / 4] = {{Adafruit_NeoTrellis(0x2E), Adafruit_NeoTrellis(0x2F)}};
        Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellisArray, width / 4, height / 4);
    }
    void show()
    {
        trellis.show();
    }
    void process(){

    }
};

//--------------------------------------------------------------------------------------------------------------------------
//Global Variables
Matrix matrix = Matrix();
//--------------------------------------------------------------------------------------------------------------------------
//Setup
void setup()
{
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    delay(500);
    Serial.println("Serial OK!");
    
    if (!matrix.trellis.begin())
    {
        Serial.println("Critical: Matrix Failed to Initialize!!");
    }
    //make sure matrix is working properly
    for (int i = 0; i < matrix.numPixels; i++)
    {
        matrix.trellis.setPixelColor(i, 0x000000);
        matrix.trellis.show();
        delay(50);
    }
}

//--------------------------------------------------------------------------------------------------------------------------
//Loop
void loop()
{
}


