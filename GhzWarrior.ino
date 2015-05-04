// digital warrior by Tomash Ghz
// modified by Karg (Timm Schlegelmilch)

// http://digitalwarrior.co/
// http://tomashg.com/
//
// licenced under Creative Commons Attribution-ShareAlike 4.0
// http://creativecommons.org/licenses/by-sa/4.0/

// firmware version 1.4.1

// NOTE: I could not assemble the encoders yet (they are on back order), so these functions are commented out / untested
// 
//
// Change in hardware/teensy/avr/cores/teensy3/usb_desc.h the following lines:
//  #define PRODUCT_NAME		  {'G','h','z',' ','W','a','r','r','i','o','r',' ','R','e','d'}
//  and adjust PRODUCT_NAME_LEN accordingly
//

const char*       versionNum            = "142";

#define ENCODER_DO_NOT_USE_INTERRUPTS

#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include "Adafruit_MCP23008.h"
#include <MIDIElements.h>
#include <Encoder.h>
#include <EEPROM.h>
#include <Bounce.h>
#include <SdFat.h>
#include <MIDI.h>

using namespace midi;
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);


// Teensy Pin definitions
#define           SD_CARD_SS_PIN          29                // orange cable
//#define         SD_CARD_SCK_PIN         13                // grey calbe
//#define         SD_CARD_MOSI_PIN        11                // brown cable
//#define         SD_CARD_MISO_PIN        12                // red calbe

//#define         MIDI_DIN_SEND_PIN       1                 // TX pin -> Serial1
//#define         SDA_PIN                 18                // I2C bus to MCP23017 and MCP23008
//#define         SCL_PIN                 19                // I2C bus to MCP23017 and MCP23008
#define           LCD_LATCH_PIN           21                // LCD: Pin connected to ST_CP of 74HC595
#define           LCD_CLOCK_PIN           22                // LCD: Pin connected to SH_CP of 74HC595
#define           LCD_DATA_PIN            20                // LCD: Pin connected to DS of 74HC595

#define           TRACKS_BUTTON_PIN       33                // yellow cable, formerly was pin 13               
#define           SEQ_BUTTON_PIN          32                // orange cable, formerly was pin 12 
#define           SHIFT_BUTTON_PIN        0                 // the small black round one
#define           EDIT_BUTTON_PIN         23

#define           OCT_POT_PIN             14                // A0: Octave potentiometer
#define           NOTE_POT_PIN            15                // A1: Note potentiometer
#define           CHANCE_POT_PIN          17                // A4: Chance potentiometer
#define           VEL_POT_PIN             16                // A2: Velocity potentiometer
#define           ENCA_PIN1               8
#define           ENCA_PIN2               7
#define           ENCB_PIN1               10
#define           ENCB_PIN2               9
//#define         ENCX_BUTTON_PIN         2                 // which encoder (X=A/B)
//#define         ENCX_BUTTON_PIN         31                // which encoder (X=A/B), red cable, formerly was pin 11
//#define         ENCX_YLED_PIN           3                 // which encoder (X=A/B), which color (Y=R/G)
#define         ENCX_YLED_PIN           4                 // which encoder (X=A/B), which color (Y=R/G)
//#define         ENCX_YLED_PIN           5                 // which encoder (X=A/B), which color (Y=R/G)
//#define         ENCX_YLED_PIN           6                 // which encoder (X=A/B), which color (Y=R/G)


// MCP23008 Pin definitions
#define           BUTTON_ROW_1            5
#define           BUTTON_ROW_2            4
#define           BUTTON_ROW_3            7
#define           BUTTON_ROW_4            6
#define           BUTTON_COL_1            0
#define           BUTTON_COL_2            1
#define           BUTTON_COL_3            2
#define           BUTTON_COL_4            3


// MCP23017 Pin definitions
#define           LEDMATRIX_GND_1         13                // ground pins - columns
#define           LEDMATRIX_GND_2         12
#define           LEDMATRIX_GND_3         15
#define           LEDMATRIX_GND_4         14
#define           LEDMATRIX_RED_1         0                 // red pins - rows
#define           LEDMATRIX_RED_2         3
#define           LEDMATRIX_RED_3         6
#define           LEDMATRIX_RED_4         9
#define           LEDMATRIX_GRN_1         1                 // green pins - rows
#define           LEDMATRIX_GRN_2         4
#define           LEDMATRIX_GRN_3         7
#define           LEDMATRIX_GRN_4         10                // blue pins - rows
#define           LEDMATRIX_BLU_1         2
#define           LEDMATRIX_BLU_2         5
#define           LEDMATRIX_BLU_3         8
#define           LEDMATRIX_BLU_4         11


// macros for the different pages
#define           DURATION_PAGE           0                 // set the note length of the current track to 16th 8th, quarter of half notes
#define           TRANSP_PAGE             1                 // set the root note (in edit mode, it is the current track, outside of edit mode, it is all tracks)
#define           SCALE_PAGE              2                 // set the scale, this changes wich notes can be set with the potentiometer
#define           CHANNEL_PAGE            3                 // select the midi channels here
#define           SONG_PAGE               4                 // the songs page
#define           TRACKS_PAGE             5                 // the tracks page
#define           SEQ_PAGE_1              6                 // page one of the sequencer (1-16)
#define           SEQ_PAGE_2              7                 // page two of the sequencer (17-32)
// shift button does not trigger a page, it sets isShift = true which then calls shiftPage()
// edit button does not trippger a page, it sets stepEditMode = true

// Feature definitions
#define           VOICES                  4                 // max 4    or   8
#define           PATTERNS                4                 // max 48        32
#define           STEPS                   32                // max 64        64


// SD Card Variables
const int         chipSelect            = SD_CARD_SS_PIN;
SdFat             sd;
SdFile            file;
boolean           sdPresent             = false;
char              fileName[13];


// Butonpad Variables
#define           BOUNCE_DELAY            24                //26 //30
#define           BUTTONPAD_DEVICE_NUM    0                 // I2C device number
Adafruit_MCP23008 _buttonpad;
int               buttonRow[]           = {BUTTON_ROW_1, BUTTON_ROW_2, BUTTON_ROW_3, BUTTON_ROW_4};
int               buttonCol[]           = {BUTTON_COL_1, BUTTON_COL_2, BUTTON_COL_3, BUTTON_COL_4};
boolean           buttons[4][4];                            // stores the button state
boolean           buttonsLast[4][4];                        // button state in the last cycle (used to evaluate changes)
unsigned long     buttonsBounce[4][4];                      // debounce timers for each button


// LED Matrix Variables
#define           LEDMATRIX_DEVICE_NUM    1                 // I2C device number
Adafruit_MCP23017 _ledmatrix;
int               ledmatrixBuffer[4][4];                    // store the values that are displayed
int               ledGroundPins[4]      = {LEDMATRIX_GND_1, LEDMATRIX_GND_2, LEDMATRIX_GND_3, LEDMATRIX_GND_4};
int               ledRedPins[4]         = {LEDMATRIX_RED_1, LEDMATRIX_RED_2, LEDMATRIX_RED_3, LEDMATRIX_RED_4};
int               ledGreenPins[4]       = {LEDMATRIX_GRN_1, LEDMATRIX_GRN_2, LEDMATRIX_GRN_3, LEDMATRIX_GRN_4};
int               ledBluePins[4]        = {LEDMATRIX_BLU_1, LEDMATRIX_BLU_2, LEDMATRIX_BLU_3, LEDMATRIX_BLU_4};

// display pages
int               displayPages[8][4][4];
Bounce            shiftButton           = Bounce(SHIFT_BUTTON_PIN, BOUNCE_DELAY);    // page shift button
int               page                  = 6;
boolean           isShift               = false;

// colors
#define           OFF                     0
#define           RED                     1
#define           GREEN                   2
#define           YELLOW                  3
#define           BLUE                    4
#define           PINK                    5
#define           CYAN                    6
#define           WHITE                   7
int               pageColor[8]          = {GREEN, YELLOW, BLUE, RED, GREEN, GREEN, GREEN, GREEN};   //*******

int               midiChannel           = 2;                // midi channel number   ******
int               controlChannel        = 13;               // midi channel number    *******
int               sequencerChannel      = 2;                // midi channel number  *******
boolean           secondary             = false;            // enable secondary midi messages  ***********
int               encodersBanked        = 1;                // *********
int               stepLength            = 1;
boolean           stackedSteps          = true;
boolean           externalClock         = false;


// Potentiometer stuff
Potentiometer     *pot1;                                    // octave
int               octavePotValue        = 0;
boolean           octavePotTakeover     = false;
boolean           octavePotChange       = false;
Potentiometer     *pot2;                                    // note
int               notePotValue          = 0;
boolean           notePotTakeover       = false;
boolean           notePotChange         = false;
Potentiometer     *pot3;                                    // velocity
int               velocityPotValue      = 0;
boolean           velocityPotTakeover   = false;
boolean           velocityPotChange     = false;
Potentiometer     *pot4;                                    // ptobability
int               probPotValue          = 0;
boolean           probPotTakeover       = false;
boolean           probPotChange         = false;

boolean           sentPreviewNote       = false;


// Karg: Button            *encBut[8][2];
// Karg: RGLed             *encLed[8][2];
Encoder           encA(ENCA_PIN1, ENCA_PIN2);               // BPM Encoder
Encoder           encB(ENCB_PIN1, ENCB_PIN2);
unsigned long int encNewA[8]            = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long int encOldA[8]            = {999, 999, 999, 999, 999, 999, 999, 999};
unsigned long int encNewB[8]            = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long int encOldB[8]            = {999, 999, 999, 999, 999, 999, 999, 999};
int               encoderLedValueA[8]   = {0, 0, 0, 0, 0, 0, 0, 0}; //store the dispalyed LED values
int               encoderLedValueB[8]   = {0, 0, 0, 0, 0, 0, 0, 0};
float             encLedOffset          = 4;
int               displayEnc            = 0;                // 0 none, 1 enc A, 2 enc b, 3 both
unsigned long     displayTimer          = 0;


// step sequencer variables
boolean           sequencerPaused       = true;
int               selectedVoice         = 0;
int               currentStep           = 0;
int               steps                 = 32;               // number of steps actively used ****   WHAT EXACTLY ARE THESE?
int               counter               = 0;

byte              clockCounter          = 0;
byte              CLOCK                 = 248;
byte              START                 = 250;
byte              CONTINUE              = 251;
byte              STOP                  = 252;

boolean           setupMode             = false;
int               setupPage             = 0;

// for midi din out
boolean           midiThrough           = true;
boolean           sequencerMidiOut      = true;
boolean           standaloneSequencer   = true;
float             bpm                   = 120;
boolean           bpmChange             = false;
int               bpmChangeTime         = 0;

boolean           internalSequencer     = true;
boolean           internalSequencerPlaying = true;
int               sequencerInterval     = (60000.0 / bpm / 24.0) * 1000;

int               selectedStep          = 0;
Bounce            editButton            = Bounce(EDIT_BUTTON_PIN, BOUNCE_DELAY);
Bounce            seqButton             = Bounce(SEQ_BUTTON_PIN, BOUNCE_DELAY);
Bounce            trackButton           = Bounce(TRACKS_BUTTON_PIN, BOUNCE_DELAY);
//Bounce            egatoButton           = Bounce(2, 30);
//Bounce            editButton2           = Bounce(16, BOUNCE_DELAY);
boolean           stepEditMode          = false;


//melodic 4 voice sequencer.
int               msSelectedEditPattern = 0;
int               msSelectedSequence    = 0;
int               msSelectedVoice     = 0;
int               msRootNote[VOICES];
int               msCurrentStep[VOICES];
int               msDirection[VOICES];
int               msStepLength[VOICES];
boolean           msDirAscending[VOICES];
int               msChannel[VOICES];
int               msLength[VOICES];
int               msSelectedPattern[VOICES];
boolean           msHasPattern[VOICES][PATTERNS];
boolean           msChannelHasPattern[VOICES];              // has this voice+pattern andy notes set? I guess the array are x/y, meaning voice/pattern
boolean           msLastPlayed[VOICES];
int               msLastPlayedNote[VOICES];
boolean           msMuted[VOICES];
boolean           msStepState[VOICES][PATTERNS][STEPS];     // [voice][pattern][step]
byte              msStepNote[VOICES][PATTERNS][STEPS];      // only 7 bits are used for MIDI notes, so the highest bit could be used to store a boolean variable (WAS 36 instead of 32, WHY????)                            
byte              msStepVelocity[VOICES][PATTERNS][STEPS];  // same here
byte              msStepChance[VOICES][PATTERNS][STEPS];    // same here
boolean           msStepLegato[VOICES][PATTERNS][STEPS];
boolean           stepPressed          = false;
int               stepPressedNum       = 0;
boolean           msLastTie[VOICES];

int               msPreviewNote        = 0;
int               msPreviewChannel     = 0;
int               msPreviewTime        = 0;
boolean           msPlayingPreview     = false;

IntervalTimer     clockTimer;
IntervalTimer     lcdTimer;

char              scaleNotes[12][12]   = {{0, 2, 3, 5, 7, 9, 11},      // melodic min
  {0, 2, 3, 5, 7, 8, 11},      // harmonic minor
  {0, 2, 3, 5, 7, 8, 10},      // natural minor
  {0, 2, 4, 5, 7, 9, 11},      // major
  {0, 2, 4, 6, 8, 10, 11},     // harmonic major
  {0, 3, 5, 7, 10},            //minor pentatonic
  {0, 2, 4, 7, 9},             //major pentatonic
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, // chromatic
  {0, 3, 5, 6, 7, 10},         // blues scale
  {0, 2, 3, 5, 6, 8, 9, 11},   // diminished
  {0, 2, 4, 6, 8, 10},         // whole tone
  {0, 2, 4, 5, 7, 9, 10, 11}
};        // bebop dominant

int               msScaleLength[12]    = {7, 7, 7, 7, 7, 5, 5, 12, 6, 8, 6, 8};
int               msCurrentScale[4];


/* LCD stuff:
https://www.sparkfun.com/products/12710

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
char              sevencodes[]         = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00, 0x63, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x30, 0x1E, 0x72, 0x38, 0x55, 0x54, 0x5C, 0x73, 0x67, 0x50, 0x2D, 0x78, 0x1C, 0x1C, 0x2A, 0x49, 0x6E, 0x1B};
// original: char sevencodes[]         = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00,0x63,0x77,0x7C,0x39,0x5E,0x79,0x71,0x3D,0x76,0x30,0x1E,0x72,0x38,0x55,0x54,0x5C,0x73,0x67,0x50,0x2D,0x78,0x1C,0x14,0x2A,0x49,0x6E,0x1B};
char              noteStrings[12][3]   = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};
char              scalesStrings[12][5] = {"melm", "harm", "natm", "majr", "har", "penm", "pent", "chro", "blue", "dimn", "whot", "bebp"};
int               a                    = 0;

int               digit[]              = {0, 0, 0, 0};
boolean           showDigit[]          = {false, false, false, false};
boolean           decimal              = false;
byte              song                 = 0;




// #####################################################################################################################
void playPreviewNote(int noteX, int velX, int chanX) {
  if (msPlayingPreview) {
    if (noteX != msPreviewNote) {
      MIDI.sendNoteOff(msPreviewNote, 0, msPreviewChannel);
      usbMIDI.sendNoteOff(msPreviewNote, 0, msPreviewChannel);
      msPlayingPreview = true;
      msPreviewNote = noteX;
      msPreviewChannel = chanX;
      msPreviewTime = millis();
      MIDI.sendNoteOn(noteX, velX, chanX);
      usbMIDI.sendNoteOn(noteX, velX, chanX);
    }
  }
  else {
    msPlayingPreview = true;
    msPreviewNote = noteX;
    msPreviewChannel = chanX;
    msPreviewTime = millis();
    MIDI.sendNoteOn(noteX, velX, chanX);
    usbMIDI.sendNoteOn(noteX, velX, chanX);
  }
}


// #####################################################################################################################
boolean noteInScale(int n, int s) { // check if a given note interval is in a scale
  n = n % 12;
  boolean flag = false;
  for (int i = 0; i < msScaleLength[s]; i++)
    if (n == scaleNotes[s][i])
      flag = true;
  return flag;
}



//######################################################################################
//save pattern animation
void savePatternAnimation() {
  clearBuffer();
  for ( int i = 0; i < 16; i++) {
    setBufferPixel(RED, i);
    printBuffer();
  }
  printBuffer();
  delay(5);
  clearBuffer();
}





