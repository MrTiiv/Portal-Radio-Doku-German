                                                 
 /***********************************************************************************************
 *                                                                                             *
 *    _|      _|            _|_|_|_|_|  _|  _|                                                 *
 *   _|_|  _|_|  _|  _|_|      _|              _|      _|                                      * 
 *   _|  _|  _|  _|_|          _|      _|  _|  _|      _|                                      *
 *   _|      _|  _|            _|      _|  _|    _|  _|                                        *
 *   _|      _|  _|            _|      _|  _|      _|                                          *      
 *                                                                                             *
 *                                                                                             *
 *  Project:       Working Portal Radio, plays Music from the Game                             *
 *  Author:        MrTiiv                                                                      *
 *  Year:          2025                                                                        *
 *  License:       GNU General Public License v3.0                                             *
 *                                                                                             *
 *  The corresponding circuit schematic is available on my GitHub:                             *
 *  https://github.com/MrTiiv/Portal-Radio-Doku-German                                         *
 *                                                                                             *
 *  This program is free software: you can redistribute it and/or modify                       *
 *  it under the terms of the GNU General Public License as published by                       *
 *  the Free Software Foundation, either version 3 of the License, or                          *
 *  (at your option) any later version.                                                        *
 *                                                                                             *
 *  This program is distributed in the hope that it will be useful,                            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                             *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU                           *
 *  General Public License for more details.                                                   *
 *                                                                                             *
 *  You should have received a copy of the GNU General Public License                          *
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.                      *
 *                                                                                             *
 *  AI Disclaimer:                                                                             *
 *  This code was written by me, MrTiiv. However, I used AI tools for assistance with the      *
 *  comments and for answering questions I had during the development process. Specifically:   *
 *    - GitHub Copilot was used to help refine and improve the comments in the code.           *
 *    - ChatGPT was used to answer questions and improve the comments in the code.             *
 *  None of the AI tools wrote any part of the code itself.                                    *
 *                                                                                             *
 ***********************************************************************************************/


// ============================
// MP3 FILE ORDER AND NAMES
// ============================
// 0001 = Aperture Science Theme
// 0002 = Portal Radio Song Decompressed
// 0003 = Portal Radio Song 8 Bit Version
// 0004 = Portal Still Alive Song
// 0005 = Portal 2 End Credits Song
// 0006 = Portal 2 Cara Mia Addio Song
// 0007 = Portal 2 OST Song 9999999
// 0008 = Aperture Desk Job Cave Jonhson
// 0009 = Portal 2 Secret SSTV Image
// 0010 = MEME SONG FLINT AND STEEL  

// ============================
// Include Required Libraries
// ============================
// Libraries required for managing the 7-segment display, serial communication and interacting with the DFPlayer mini module.
#include <TM1637.h>                 // 7-Segment Display Library
#include <SoftwareSerial.h>         // Software UART for DFPlayer Mini
#include "Arduino.h"                // Core Arduino functions
#include "DFRobotDFPlayerMini.h"    // DFPlayer Mini Library

// ============================
// DFPlayer Mini Setup
// ============================
// Initializes the software serial communication for the DFPlayer Mini module.
SoftwareSerial softSerial(/*rx =*/10, /*tx =*/11);  // RX and TX pins for software serial communication with DFPlayer
#define FPSerial softSerial
DFRobotDFPlayerMini myDFPlayer;

// ============================
// GPIO Pin Definitions
// ============================
// Define the GPIO Pins for the 7-segment display, status LED and rotary encoder.
TM1637 tm(6, 7);             // TM1637 Display: CLK = Pin 6, DIO = Pin 7
#define FRONT_LED        8   // Status LED
#define ENCODER_CLK      2   // Rotary encoder - CLK pin
#define ENCODER_DT       3   // Rotary encoder - DT pin
#define ENCODER_BUTTON   4   // Rotary encoder - Button pim

// ============================
// Global Variables
// ============================
// Storage for rotary encoder position, rotary encoder button states and debounce timing
// These variables are used to manage the rotary encoder and LED blinking behavior. 
int static rotaryNum = 0;           // Rotary Encoder Position
int static buttonPushed;            // Rotary Encoder Button press state
static bool buttonReleased = true;  // Rotary Encoder Button release detection
int static lastCLKState;            // Last CLK state for rotary encoder rotation detection
unsigned long lastDebounceTime = 0; // Millis storage for Debounce timer rotary encoder

// LED Variable: Used for blinking the FRONT_LED in "MODE_SELECT"
unsigned long millisLEDBlinkTime = 0;   

// Mode Constants: Define different modes.
int static modeNumber;                 // Active Mode Selection
int const  MODE_SELECT         = 1;    // Mode: Selection Menu
int const  MODE_DEFAULT_FREQ   = 2;    // Mode: Default frequency 85.2 
int const  MODE_FREQ_2         = 3;    // Mode: Frequency 88.1
int const  MODE_FREQ_3         = 4;    // Mode: Frequency 90.5
int const  MODE_FREQ_4         = 5;    // Mode: Frequency 92.3
int const  MODE_FREQ_5         = 6;    // Mode: Frequency 94.7
int const  MODE_FREQ_6         = 7;    // Mode: Frequency 96.8
int const  MODE_FREQ_7         = 8;    // Mode: Frequency 97.6
int const  MODE_FREQ_8         = 9;    // Mode: Frequency 99.9
int const  MODE_FREQ_9         = 10;   // Mode: Frequency 102.4

// ============================
// Setup Function
// ============================
// Initializes serial communication, GPIO pins, TM1637 display and the DFPlayer Mini module. 
void setup() {
    softSerial.begin(9600); // Start serial communication for DFPlayer Mini
    Serial.begin(115200);   // Start serial communication for debugging

    // Configure GPIO pins for output and input
    pinMode(FRONT_LED, OUTPUT);
    pinMode(ENCODER_CLK, INPUT);
    pinMode(ENCODER_DT, INPUT);
    pinMode(ENCODER_BUTTON, INPUT);

    // Initialize TM1637 Display
    tm.begin();
    tm.setBrightnessPercent(90);    // Set display brightness
    tm.display("     ");            // Clear Display on Startup

    // Read initial state of the rotary encoder
    lastCLKState = digitalRead(ENCODER_CLK);

    // Initialize DFPlayer Mini
     if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  
     }
    myDFPlayer.volume(30);  // Set DFPlayer volume
    myDFPlayer.play(1);     // Play startup track

    startup();      // Run startup animation
}

// ============================
// Startup Animation
// ============================
// Simulates radio scanning effect on the 7-segment display.
// The display shows random numbers before settling on the default frequency
// Plays the Aperture Science Theme during startup
void startup() {
    String strRandNumber1;      // First part of the random number
    String strRandNumber2;      // Second part of the random number

    for (unsigned counter = 0; counter < 1000; ++counter) {
        // Random Numbers 4-Digits
        if (counter < 50) { 
            strRandNumber1 = random(99);
            strRandNumber2 = random(99);
            String strNum1Num2 = strRandNumber1 + "." + strRandNumber2;
            tm.display(strNum1Num2);
            delay(50);
            continue;
        }
        // Random Numbers 3-Digitls
        else if (counter < 100) { 
            strRandNumber1 = random(99);
            strRandNumber2 = random(9);
            String strNum1Num2 = strRandNumber1 + "." + strRandNumber2 + " ";
            tm.display(strNum1Num2);
            delay(50);
            continue;
        }
        else{
            tm.display("85.2"); // Final display after animation
        }
    }

    modeNumber = MODE_DEFAULT_FREQ;  // Set default mode
    myDFPlayer.loop(2);              // Start looping track 2 after startup
}

// ============================
// Main Loop Function
// ============================
// Continuously checks the current mode and executes the appropriate function.
// Blinks the LED in "MODE_SELECT" mode.
void loop() {
    switch(modeNumber) {
        case MODE_SELECT:          Select();        break;
        case MODE_DEFAULT_FREQ:    DefaultFreq();   break;
        case MODE_FREQ_2:          Freq_2();        break;
        case MODE_FREQ_3:          Freq_3();        break;
        case MODE_FREQ_4:          Freq_4();        break;
        case MODE_FREQ_5:          Freq_5();        break;
        case MODE_FREQ_6:          Freq_6();        break;
        case MODE_FREQ_7:          Freq_7();        break;
        case MODE_FREQ_8:          Freq_8();        break;
        case MODE_FREQ_9:          Freq_9();        break;
    }

    // Blink LED when in MODE_SELECTion mode
    const int MODE_SELECTorLEDBlink = 250; // LED Blink Speed
    if(modeNumber == MODE_SELECT) {
        if(millis() - millisLEDBlinkTime > MODE_SELECTorLEDBlink) {
            if(digitalRead(FRONT_LED) == LOW) {
                digitalWrite(FRONT_LED, HIGH);
            }
            else{
                digitalWrite(FRONT_LED, LOW);
            }
            millisLEDBlinkTime = millis();
        }
    }
}

// ============================
// Rotary Encoder Logic
// ============================
// Reads encoder states to detect rotation direction and updates mode selection.
void RotaryEncoder() {
    int intCurrentCLKState = digitalRead(ENCODER_CLK);
    int intCurrentDTState = digitalRead(ENCODER_DT);
    const int constDebounceDelay = 2; // Debounce delay in millisecond 

    if (intCurrentCLKState != lastCLKState) {
        if (millis() - lastDebounceTime > constDebounceDelay ) {
            // Determine direction of encoder rotation
            if (intCurrentCLKState == LOW && intCurrentDTState != intCurrentCLKState) {
                rotaryNum++; // Clockwise rotation
            }
                else if (intCurrentCLKState == LOW && intCurrentDTState == intCurrentCLKState) {
                rotaryNum--; //Counter-clockwise rotation
                }
        }
        lastDebounceTime = millis();
    }
    lastCLKState = intCurrentCLKState;

    // Wrap-around for rotary encoder position
    if (rotaryNum < 0) {
        rotaryNum = 8;
    }
    else if (rotaryNum > 8) {
        rotaryNum = 0;
    }
}

// ============================
// Mode Selection Menu
// ============================
// Allows the user to select the desired radio frequency using the rotary encoder.
// Updated the mode and starts playing the corresponding track
void Select() {
    buttonPushed = digitalRead(ENCODER_BUTTON);
    if(buttonReleased && buttonPushed == HIGH){
        buttonReleased = false;
    }

    // Display the frequency and handle button press to select the mode
    switch(rotaryNum) {
        case 0:
            tm.display("85.2 ");        // Display selected frequency
                if((buttonPushed == LOW) && (!buttonReleased)){
                modeNumber = MODE_DEFAULT_FREQ;
                myDFPlayer.loop(2);     // Loop selected Song
                buttonReleased = true;
            }
            break;
        case 1:
            tm.display("88.1 ");        // Display selected frequency
            if((buttonPushed == LOW) && (!buttonReleased)){
                modeNumber = MODE_FREQ_2;
                myDFPlayer.loop(3);     // Loop selected Song
                buttonReleased = true;
            }
            break;
        case 2:
            tm.display("90.5 ");        // Display selected frequency
            if((buttonPushed == LOW) && (!buttonReleased)){
                modeNumber = MODE_FREQ_3;
                myDFPlayer.loop(4);     // Loop selected Song
                buttonReleased = true;
            }
            break;
        case 3:
            tm.display("92.3 ");        // Display selected frequency
            if((buttonPushed == LOW) && (!buttonReleased)){
                modeNumber = MODE_FREQ_4;
                myDFPlayer.loop(5);     // Loop selected Song
                buttonReleased = true;
            }
            break;
        case 4:
            tm.display("94.7 ");        // Display selected frequency
            if((buttonPushed == LOW) && (!buttonReleased)){
                modeNumber = MODE_FREQ_5;
                myDFPlayer.loop(6);     // Loop selected Song
                buttonReleased = true;
            }
            break;
        case 5:
            tm.display("96.8 ");        // Display selected frequency
            if((buttonPushed == LOW) && (!buttonReleased)){
                modeNumber = MODE_FREQ_6;
                myDFPlayer.loop(9);     // Loop selected Song
                buttonReleased = true;
            }
            break;
        case 6:
            tm.display("97.6 ");        // Display selected frequency
            if((buttonPushed == LOW) && (!buttonReleased)){
                modeNumber = MODE_FREQ_7;
                myDFPlayer.loop(8);     // Loop selected Song
                buttonReleased = true;
            }
            break;
        case 7:
            tm.display("99.9 ");        // Display selected frequency
                if((buttonPushed == LOW) && (!buttonReleased)){
                modeNumber = MODE_FREQ_8;
                myDFPlayer.loop(7);     // Loop selected Song
                buttonReleased = true;
            }
            break;
        case 8:
            tm.display("102.4");        // Display selected frequency
            if((buttonPushed == LOW) && (!buttonReleased)){
                modeNumber = MODE_FREQ_9;
                myDFPlayer.loop(10);    // Loop selected Song
                buttonReleased = true;
            }
            break;

        // If case is unknown, Display 8888
        default:
            tm.display("8888");
            break;
    }

    // Check for rotary movement
    RotaryEncoder(); 
}

// ============================
// Frequency Modes
// ============================
// These functions represent each of the frequency modes.
// Each frequency is displayed on the 7-segment display, and the corresponding track is played on the DFPlayer Mini.
// The LED stays on during playback, and pressing the encoder button stops the playback, returning to the Selection Mode. 

void DefaultFreq() {
    buttonPushed = digitalRead(ENCODER_BUTTON);
        digitalWrite(FRONT_LED, HIGH);  // Turn on LED
        tm.display("85.2 ");            // Display current frequency      
    if(buttonReleased && buttonPushed == HIGH) {
        buttonReleased = false;
    }
    if(!buttonReleased && buttonPushed == LOW) {
        modeNumber = MODE_SELECT;
        buttonReleased = true;
        digitalWrite(FRONT_LED, LOW);   // Turn off LED
        myDFPlayer.stop();              // Stop the DFPlayer
    }
}
void Freq_2() {
    buttonPushed = digitalRead(ENCODER_BUTTON);
        digitalWrite(FRONT_LED, HIGH);  // Turn off LED
        tm.display("88.1 ");            // Display current frequency
    if(buttonReleased && buttonPushed == HIGH) {
        buttonReleased = false;
    }
    if(!buttonReleased && buttonPushed == LOW) {
        modeNumber = MODE_SELECT;
        buttonReleased = true;
        digitalWrite(FRONT_LED, LOW);   // Turn off LED
        myDFPlayer.stop();              // Stop the DFPlayer
    } 
}
void Freq_3() {
    buttonPushed = digitalRead(ENCODER_BUTTON);
        digitalWrite(FRONT_LED, HIGH);  // Turn on LED
        tm.display("90.5 ");            // Display current frequency
    if(buttonReleased && buttonPushed == HIGH) {
        buttonReleased = false;
    }
    if(!buttonReleased && buttonPushed == LOW) {
        modeNumber = MODE_SELECT;
        buttonReleased = true;
        digitalWrite(FRONT_LED, LOW);   // Turn off LED
        myDFPlayer.stop();              // Stop the DFPlayer
    }
}
void Freq_4() {
    buttonPushed = digitalRead(ENCODER_BUTTON);
        digitalWrite(FRONT_LED, HIGH);  // Turn on LED
        tm.display("92.3 ");            // Display current frequency
    if(buttonReleased && buttonPushed == HIGH) {
        buttonReleased = false;
    }
    if(!buttonReleased && buttonPushed == LOW) {
        modeNumber = MODE_SELECT;
        buttonReleased = true;
        digitalWrite(FRONT_LED, LOW);   // Turn off LED
        myDFPlayer.stop();              // Stop the DFPlayer
    }
}
void Freq_5() {
    buttonPushed = digitalRead(ENCODER_BUTTON);
        digitalWrite(FRONT_LED, HIGH);  // Turn on LED
        tm.display("94.7 ");            // Display current frequency
    if(buttonReleased && buttonPushed == HIGH) {
        buttonReleased = false;
    }
    if(!buttonReleased && buttonPushed == LOW) {
        modeNumber = MODE_SELECT;
        buttonReleased = true;
        digitalWrite(FRONT_LED, LOW);   // Turn off LED
        myDFPlayer.stop();              // Stop the DFPlayer
    }
}
void Freq_6() {
    buttonPushed = digitalRead(ENCODER_BUTTON);
        digitalWrite(FRONT_LED, HIGH);  // Turn on LED
        tm.display("96.8 ");            // Display current frequency
    if(buttonReleased && buttonPushed == HIGH) {
        buttonReleased = false;
    }
    if(!buttonReleased && buttonPushed == LOW) {
        modeNumber = MODE_SELECT;
        buttonReleased = true;
        digitalWrite(FRONT_LED, LOW);   // Turn off LED
        myDFPlayer.stop();              // Stop the DFPlayer
    }
}
void Freq_7() {
    buttonPushed = digitalRead(ENCODER_BUTTON);
        digitalWrite(FRONT_LED, HIGH);  // Turn on LED
        tm.display("97.6 ");            // Display current frequency
    if(buttonReleased && buttonPushed == HIGH) {
        buttonReleased = false;
    }
    if(!buttonReleased && buttonPushed == LOW) {
        modeNumber = MODE_SELECT;
        buttonReleased = true;
        digitalWrite(FRONT_LED, LOW);   // Turn off LED
        myDFPlayer.stop();              // Stop the DFPlayer
    }
}
void Freq_8() {
    buttonPushed = digitalRead(ENCODER_BUTTON);
        digitalWrite(FRONT_LED, HIGH);  // Turn on LED
        tm.display("99.9 ");            // Display current frequency
    if(buttonReleased && buttonPushed == HIGH) {
        buttonReleased = false;
    }
    if(!buttonReleased && buttonPushed == LOW) {
        modeNumber = MODE_SELECT;
        buttonReleased = true;
        digitalWrite(FRONT_LED, LOW);   // Turn off LED
        myDFPlayer.stop();              // Stop the DFPlayer
    }
}
void Freq_9() {
    buttonPushed = digitalRead(ENCODER_BUTTON);
        digitalWrite(FRONT_LED, HIGH);  // Turn on LED
        tm.display("102.4");            // Display current frequency
    if(buttonReleased && buttonPushed == HIGH) {
        buttonReleased = false;
    }
    if(!buttonReleased && buttonPushed == LOW) {
        modeNumber = MODE_SELECT;
        buttonReleased = true;
        digitalWrite(FRONT_LED, LOW);   // Turn off LED
        myDFPlayer.stop();              // Stop the DFPlayer
    }  
}


//                .,-:;//;:=,
//            . :H@@@MM@M#H/.,+%;,
//         ,/X+ +M@@M@MM%=,-%HMMM@X/,            
//       -+@MM; $M@@MH+-,;XMMMM@MMMM@+-          
//      ;@M@@M- XM@X;. -+XXXXXHHH@M@M#@/.         
//    ,%MM@@MH ,@%=             .---=-=:=,.    
//    =@#@@@MX.,                -%HX$$%%%:;       
//   =-./@M@M$                   .;@MMMM@MM:
//   X@/ -$MM/                    . +MM@@@M$
//  ,@M@H: :@:                    . =X#@@@@-
//  ,@@@MMX, .                    /H- ;@M@M=
//  .H@@@@M@+,                    %MM+..%#$.
//   /MMMM@MMH/.                  XM@MH; =;
//   /%+%$XHH@$=              , .H@@@@MX,
//     .=--------.           -%H.,@@@@@MX,
//     .%MM@@@HHHXX$$$%+- .:$MMX =M@@MM%.
//       =XMMM@MM@MM#H;,-+HMM@M+ /MMMX=
//         =%@M@M#@$-.=$@MM@@@M; %M%=
//           ,:+$+-,/H#MMMMMMM@= =,
//                 =++%%%%+/:-.





