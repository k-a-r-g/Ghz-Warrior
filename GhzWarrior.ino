// digital warrior by Tomash Ghz
// modified by Karg (Timm Schlegelmilch)

// http://digitalwarrior.co/
// http://tomashg.com/
//
// licenced under Creative Commons Attribution-ShareAlike 4.0
// http://creativecommons.org/licenses/by-sa/4.0/

// firmware version 1.4

// TO DO: make songs page, 3 songs available and a save button.
// dump EEPROM to SD card, get EEPROM from SD card (skip EEPROM access?)



#define ENCODER_DO_NOT_USE_INTERRUPTS

#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include "Adafruit_MCP23008.h"
#include <MIDIElements.h>
#include <Encoder.h>
#include <EEPROM.h>
#include <Bounce.h>
#include <MIDI.h>

// Butonpad Variables
Adafruit_MCP23008 _buttonpad;
int               buttonRow[]          = {5, 4, 7, 6};
int               buttonCol[]          = {0, 1, 2, 3};
int               buttons[4][4];
unsigned long     buttonsBounce[4][4];

// Display Variables
Adafruit_MCP23017 _display;
int               dispGroundPins[4]    = {13, 12, 15, 14}; // ground pins - columns
int               dispRedPins[4]       = {0, 3, 6, 9};     // red pins - rows
int               dispGreenPins[4]     = {1, 4, 7, 10};    // reen pins - rows
int               dispBluePins[4]      = {2, 5, 8, 11};    // blue pins - rows
int               displayBuffer[4][4];                     // store the values that are displayed

// display pages
int               displayPages[8][4][4];
unsigned int      bounceDelay          = 24;               //26 //30
Bounce            pageShift            = Bounce(0, 30);    // page shift button
int               page                 = 6;
boolean           pageSelect           = false;

// colors
const int         OFF                  = 0;
const int         RED                  = 1;
const int         GREEN                = 2;
const int         YELLOW               = 3;
const int         BLUE                 = 4;
const int         PINK                 = 5;
const int         CYAN                 = 6;
const int         WHITE                = 7;
int               pageColor[8]         = {GREEN, YELLOW, BLUE, RED, GREEN, GREEN, GREEN, GREEN};   //*******

int               midiChannel          = 1;                // midi channel number   ******
int               controlChannel       = 2;                // midi channel number    *******
int               sequencerChannel     = 3;                // midi channel number  *******
boolean           debug                = false;            // print to serial instead of midi
boolean           secondary            = false;            // enable secondary midi messages  ***********
boolean           abletonEncoder       = false;            //      *******
int               encodersBanked       = 1;                // *********
int               stepLength           = 1;
boolean           stackedSteps         = true;
boolean           externalClock        = false;

Potentiometer     *pot1;                                   //(14, controlChannel, 17, false, debug); // knob on pin 45 (A7)
Potentiometer     *pot2;                                   //(15, controlChannel, 18, false, debug); // knob on pin 44 (A6)
Potentiometer     *pot3;                                   //(16, controlChannel, 19, false, debug); // knob on pin 45 (A7)
Potentiometer     *pot4;                                   //(17, controlChannel, 20, false, debug); // knob on pin 44 (A6)

Button            *encBut[8][2];
RGLed             *encLed[8][2];
Encoder           encA(8, 7);
Encoder           encB(10, 9);
unsigned long int encNewA[8]           = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long int encOldA[8]           = {999, 999, 999, 999, 999, 999, 999, 999};
unsigned long int encNewB[8]           = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long int encOldB[8]           = {999, 999, 999, 999, 999, 999, 999, 999};
int               encoderLedValueA[8]  = {0, 0, 0, 0, 0, 0, 0, 0}; //store the dispalyed LED values
int               encoderLedValueB[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
float             encLedOffset         = 4;
int               displayEnc           = 0;                // 0 none, 1 enc A, 2 enc b, 3 both
unsigned long     displayTimer         = 0;


// step sequencer variables
boolean           stepSequencer        = true;             // ****
boolean           sequencerPaused      = true;
int               selectedChannel      = 0;
int               currentStep          = 0;
int               STEPS                = 32;               // length of steps ****
int               counter              = 0;

byte              clockCounter         = 0;
byte              CLOCK                = 248;
byte              START                = 250; 
byte              CONTINUE             = 251; 
byte              STOP                 = 252; 

boolean           setupMode            = false;
int               setupPage            = 0;

// multiplexer address pins
// only for expantion board
int               muxAA                = 21;
int               muxAB                = 22;
int               muxAC                = 23;
int               muxAD                = 12;

 
///////////////////////////// for expansion board
boolean           extended             = false;
Potentiometer     *expPot[6];                              // expansion board pots

int               muxPinMatrix[16][4]  = {{LOW,LOW,LOW,LOW}, {HIGH,LOW,LOW,LOW}, {LOW,HIGH,LOW,LOW}, {HIGH,HIGH,LOW,LOW},
                                          {LOW,LOW,HIGH,LOW}, {HIGH,LOW,HIGH,LOW}, {LOW,HIGH,HIGH,LOW}, {HIGH,HIGH,HIGH,LOW},
                                          {LOW,LOW,LOW,HIGH}, {HIGH,LOW,LOW,HIGH}, {LOW,HIGH,LOW,HIGH}, {HIGH,HIGH,LOW,HIGH},
                                          {LOW,LOW,HIGH,HIGH}, {HIGH,LOW,HIGH,HIGH}, {LOW,HIGH,HIGH,HIGH}, {HIGH,HIGH,HIGH,HIGH}};
                    
// for midi din out
boolean           midiThrough          = true;
boolean           sequencerMidiOut     = true;
boolean           standaloneSequencer  = true;
float             bpm                  = 120;
boolean           bpmChange            = false;
int               bpmChangeTime        = 0;

boolean           internalSequencer    = true;
boolean           internalSequencerPlaying = true;
int               sequencerInterval    = (60000.0/bpm/24.0)*1000;

int               sellectedStep=0;
Bounce            editShift            = Bounce(11, 30);
Bounce            editShift2           = Bounce(23, 30);
Bounce            seqShift             = Bounce(12, 30);
Bounce            trackShift           = Bounce(13, 30);
//Bounce legatoButton = Bounce(2, 30);
boolean           stepEditMode         = false;

boolean           octaveKnobTakeover   = false;
boolean           noteKnobTakeover     = false;
boolean           velocityKnobTakeover = false;
boolean           sentPreviewNote      = false;
boolean           vknob                = false;
boolean           probknob             = false;

//melodic 4 voice sequencer.
int               msSelectedEditPattern= 0;
int               msSelectedSequence   = 0;
int               msSelectedChannel    = 0;
int               msRootNote[4];
int               msCurrentStep[4];
int               msDirection[4];
int               msStepLength[4];
boolean           msDirAscending[4];
int               msChannel[4];
int               msLength[4];
int               msSelectedPattern[4];
boolean           msHasPattern[4][4];
boolean           msChannelHasPattern[4];
boolean           msLastPlayed[4];
int               msLastPlayedNote[4];
boolean           msMuted[4];
boolean           msStepState[4][4][32];
int               msStepNote[4][4][36];
int               msStepVelocity[4][4][32];
int               msStepChance[4][4][32];
boolean           msStepLegato[4][4][32];
boolean           stepPressed          = false;
int               stepPressedNum       = 0;
boolean           msLastTie[4];

int               msPreviewNote        = 0;
int               msPreviewChannel     = 0;
int               msPreviewTime        = 0;
boolean           msPlayingPreview     = false;

IntervalTimer     clockTimer;
IntervalTimer     lcdTimer;

char              scaleNotes[12][12]   = {{0,2,3,5,7,9,11},            // melodic min
                                          {0,2,3,5,7,8,11},            // harmonic minor
                                          {0,2,3,5,7,8,10},            // natural minor
                                          {0,2,4,5,7,9,11},            // major
                                          {0,2,4,6,8,10,11},           // harmonic major
                                          {0,3,5,7,10},                //minor pentatonic
                                          {0,2,4,7,9},                 //major pentatonic
                                          {0,1,2,3,4,5,6,7,8,9,10,11}, // chromatic
                                          {0,3,5,6,7,10},              // blues scale
                                          {0,2,3,5,6,8,9,11},          // diminished
                                          {0,2,4,6,8,10},              // whole tone
                                          {0,2,4,5,7,9,10,11}};        // bebop dominant
                       
int               msScaleLength[12]    = {7,7,7,7,7,5,5,12,6,8,6,8};
int               msCurrentScale[4];


//lcd stuff
//Pin connected to ST_CP of 74HC595
int               latchPin             = 21;
//Pin connected to SH_CP of 74HC595
int               clockPin             = 22;
////Pin connected to DS of 74HC595
int               dataPin              = 20;

/*
index 0 : 0
index 1 : 1
index 2 : 2
index 3 : 3
index 4 : 4
index 5 : 5
index 6 : 6
index 7 : 7
index 8 : 8
index 9 : 9
index 10 :  
index 11 : #
index 12 : a
index 13 : b
index 14 : c
index 15 : d
index 16 : e
index 17 : f
index 18 : g
index 19 : h
index 20 : i
index 21 : j
index 22 : k
index 23 : l
index 24 : m
index 25 : n
index 26 : o
index 27 : p
index 28 : q
index 29 : r
index 30 : s
index 31 : t
index 32 : u
index 33 : v
index 34 : w
index 35 : x
index 36 : y
index 37 : z
*/
#define	          DOTPOINT_MASK          80
char              sevencodes[]         = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00,0x63,0x77,0x7C,0x39,0x5E,0x79,0x71,0x3D,0x76,0x30,0x1E,0x72,0x38,0x55,0x54,0x5C,0x73,0x67,0x50,0x2D,0x78,0x1C,0x14,0x2A,0x49,0x6E,0x1B};
char              noteStrings[12][3]   = {"c","c#","d","d#","e","f","f#","g","g#","a","a#","b"};
char              scalesStrings[12][5] = {"melm","harm","natm","majr","har","penm","pent","chro","blue","dimn","whot","bebp"};
int               a                    = 0;

int               digit[]              = {0,0,0,0};
boolean           showDigit[]          = {false,false,false,false};
boolean           decimal              = false;
byte              song                 = 0;


// #####################################################################################################################
void playPreviewNote(int noteX, int velX, int chanX){
  if(msPlayingPreview){
    if(noteX!=msPreviewNote){
      // (Karg) MIDI.sendNoteOff(msPreviewNote, 0, msPreviewChannel);
      usbMIDI.sendNoteOff(msPreviewNote, 0, msPreviewChannel);
      msPlayingPreview=true;
      msPreviewNote=noteX;
      msPreviewChannel=chanX;
      msPreviewTime=millis();
      // (Karg) MIDI.sendNoteOn(noteX, velX, chanX);
      usbMIDI.sendNoteOn(noteX, velX, chanX);
    }
  }
  else{
    msPlayingPreview=true;
    msPreviewNote=noteX;
    msPreviewChannel=chanX;
    msPreviewTime=millis();
    // (Karg) MIDI.sendNoteOn(noteX, velX, chanX);
    usbMIDI.sendNoteOn(noteX, velX, chanX);
  }
}


// #####################################################################################################################
boolean noteInScale(int n, int s){ // check if a given note interval is in a scale
	n = n%12;
	boolean flag = false;
	for(int i=0;i<msScaleLength[s]; i++)
		if(n==scaleNotes[s][i])
			flag=true;
	return flag;
}






//######################################################################################
//save pattern animation
void savePatternAnimation(){
  clearBuffer();
    for ( int i=0; i<16; i++) {
      setBufferPixel(RED, i);
      printBuffer();
    }
  printBuffer();
  delay(5);
  clearBuffer();
}

//#######################################################################################
//se the expansion board multiplexer channel
void setMux(int n){
    digitalWrite(muxAA,muxPinMatrix[n][0]);
    digitalWrite(muxAB,muxPinMatrix[n][1]);
    digitalWrite(muxAC,muxPinMatrix[n][2]);
    digitalWrite(muxAD,muxPinMatrix[n][3]);
}




