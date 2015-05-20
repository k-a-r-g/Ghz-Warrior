// Digital warrior by Tomash Ghz
// modified by Karg (Timm Schlegelmilch)

// http://Digitalwarrior.co/
// http://tomashg.com/
//
// licenced under Creative Commons Attribution-ShareAlike 4.0
// http://creativecommons.org/licenses/by-sa/4.0/

// NOTE: I could not assemble the encoders yet (they are on back order), so these functions are commented out / untested
// 
//
// Change in hardware/teensy/avr/cores/teensy3/usb_desc.h the following lines:
//  #define PRODUCT_NAME		  {'G','h','z',' ','W','a','r','r','i','o','r'}
//  and adjust PRODUCT_NAME_LEN accordingly
//

// ToDo:
// - write MIDI CC slide function: button1: channel, button 2: command, button 3: start (encA: beat, encB: value), button 4: stop (encA: beat, endB: value)
// - write load/save setup routine
// - TEST EEPROM ROUTINES!
// - real-time clock with MTC messages

//#define           SD_MOD                       


const char*       versionNum                 = "144";

#define ENCODER_DO_NOT_USE_INTERRUPTS

#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include "Adafruit_MCP23008.h"
#include <MIDIElements.h>
#include <Encoder.h>
#include <EEPROM.h>
#include <Bounce.h>
#include <MIDI.h>

using namespace midi;
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

#ifndef SD_MOD
  #define         TRACKS_BUTTON_PIN            13                              
  #define         SEQ_BUTTON_PIN               12                

#else
  #include <SdFat.h>

  // Teensy Pin definitions
  #define         SD_CARD_SS_PIN               29                // orange cable
  //#define       SD_CARD_SCK_PIN              13                // grey calbe
  //#define       SD_CARD_MOSI_PIN             11                // brown cable
  //#define       SD_CARD_MISO_PIN             12                // red cable
  
  #define         TRACKS_BUTTON_PIN            33                // yellow re-routing cable, formerly was pin 13               
  #define         SEQ_BUTTON_PIN               32                // orange re-routing cable, formerly was pin 12 
#endif

#define           EDIT_BUTTON_PIN              23
#define           SHIFT_BUTTON_PIN             0                 // the small black round one

#define           NOTE_POT_PIN                 14                // A0: Octave potentiometer
#define           OCT_POT_PIN                  15                // A1: Note potentiometer
#define           CHANCE_POT_PIN               17                // A4: Chance potentiometer
#define           VEL_POT_PIN                  16                // A2: Velocity potentiometer
#define           ENCA_PIN1                    8
#define           ENCA_PIN2                    7
#define           ENCB_PIN1                    10                         
#define           ENCB_PIN2                    9
//#define         ENCX_BUTTON_PIN              2                 // which encoder (X=A/B)
//#define         ENCX_BUTTON_PIN              31     KARG:           // which encoder (X=A/B), red re-routing cable, formerly was pin 11
//#define         ENCX_YLED_PIN                3                 // which encoder (X=A/B), which color (Y=R/G)
#define           ENCX_YLED_PIN                4                 // which encoder (X=A/B), which color (Y=R/G)
//#define         ENCX_YLED_PIN                5                 // which encoder (X=A/B), which color (Y=R/G)
//#define         ENCX_YLED_PIN                6                 // which encoder (X=A/B), which color (Y=R/G)

#define           LCD_LATCH_PIN                21                // LCD: Pin connected to ST_CP of 74HC595
#define           LCD_CLOCK_PIN                22                // LCD: Pin connected to SH_CP of 74HC595
#define           LCD_DATA_PIN                 20                // LCD: Pin connected to DS of 74HC595
//#define         MIDI_DIN_SEND_PIN            1                 // TX pin -> Serial1
//#define         SDA_PIN                      18                // I2C bus to MCP23017 and MCP23008
//#define         SCL_PIN                      19                // I2C bus to MCP23017 and MCP23008


// MCP23008 Pin definitions
#define           BUTTON_ROW_1                 5
#define           BUTTON_ROW_2                 4
#define           BUTTON_ROW_3                 7
#define           BUTTON_ROW_4                 6
#define           BUTTON_COL_1                 0
#define           BUTTON_COL_2                 1
#define           BUTTON_COL_3                 2
#define           BUTTON_COL_4                 3


// MCP23017 Pin definitions
#define           LEDMATRIX_GND_1              13                // ground pins - columns
#define           LEDMATRIX_GND_2              12
#define           LEDMATRIX_GND_3              15
#define           LEDMATRIX_GND_4              14
#define           LEDMATRIX_RED_1              0                 // red pins - rows
#define           LEDMATRIX_RED_2              3
#define           LEDMATRIX_RED_3              6
#define           LEDMATRIX_RED_4              9
#define           LEDMATRIX_GRN_1              1                 // green pins - rows
#define           LEDMATRIX_GRN_2              4
#define           LEDMATRIX_GRN_3              7
#define           LEDMATRIX_GRN_4              10                // blue pins - rows
#define           LEDMATRIX_BLU_1              2
#define           LEDMATRIX_BLU_2              5
#define           LEDMATRIX_BLU_3              8
#define           LEDMATRIX_BLU_4              11


// macros for the different pages
#define           DURATION_PAGE                0                 // "dur"  set the note length of the current track to 16th 8th, quarter of half notes
#define           TRANSP_PAGE                  1                 // "tran" set the root note (in edit mode, it is the current track, outside of edit mode, it is all tracks)
#define           SCALE_PAGE                   2                 // "scal" set the scale, this changes wich notes can be set with the potentiometer
#define           CHANNEL_PAGE                 3                 // "chan" select the midi channels here
#define           SEQ_PAGE_1                   4                 // "sen1" page one of the sequencer (notes 1-16)
#define           SEQ_PAGE_2                   5                 // "sen2" page two of the sequencer (notes 17-32)
#define           SEQ_PAGE_3                   6                 // "sen3" page one of the sequencer (notes 33-48)
#define           SEQ_PAGE_4                   7                 // "sen4" page two of the sequencer (notes 49-64)
#define           SONG_PAGE                    8                 // "song" songs page
#define           TRACKS_PAGE                  9                 // "trck" tracks page
#define           DIRECTION_BUTTON             11                // button to cycle through the play directions
#define           RESTART_BUTTON               12
#define           CLEAR_BUTTON                 13                // clear button
#define           MUTE_BUTTON                  14                // mute button
#define           PLAY_BUTTON                  15                // play button
int               displayPages[10][4][4];                        // ten pages, 4 ?, 4 ?
int               page                       = SEQ_PAGE_1;
unsigned long     blinkTimer                 = 0;
const int         blinkTime                  = 300;
// shift button does not trigger a page, it sets isShift = true which then calls shiftPage()
// edit button does not trippger a page, it sets stepEditMode = true


// colors
#define           OFF                          0
#define           RED                          1
#define           GREEN                        2
#define           YELLOW                       3
#define           BLUE                         4
#define           PINK                         5
#define           CYAN                         6
#define           WHITE                        7
int               pageColor[8]               = {GREEN, YELLOW, BLUE, RED, GREEN, GREEN, GREEN, GREEN};   //*******  KARG: ????


// Feature definitions
#define           VOICES                       6                 // 8   max 4   or  6
#define           PATTERNS                     24                // 24  max 32  or  24
#define           STEPS                        64                
#define           SLIDES                       32
//128 bpm -> 32 beats (16 steps; 4/4) / minute
//        -> 8 Patterns (64 steps) minute
//        -> 4 minutes with absolutely unique patterns!


// SD Card Variables
boolean           sdPresent                  = false;
#ifdef SD_MOD
  SdFat             sd;
  SdFile            file;
  char              fileName[13];
#endif


// Button Variables
#define           BOUNCE_DELAY                 24                //26 //30
Bounce            shiftButton                = Bounce(SHIFT_BUTTON_PIN, BOUNCE_DELAY);    // page shift button
boolean           isShift                    = false;
Bounce            editButton                 = Bounce(EDIT_BUTTON_PIN, BOUNCE_DELAY);
Bounce            seqButton                  = Bounce(SEQ_BUTTON_PIN, BOUNCE_DELAY);
Bounce            trackButton                = Bounce(TRACKS_BUTTON_PIN, BOUNCE_DELAY);


// Butonpad Variables
#define           BUTTONPAD_DEVICE_NUM         0                 // I2C device number
Adafruit_MCP23008 _buttonpad;
int               buttonRow[]                = {BUTTON_ROW_1, BUTTON_ROW_2, BUTTON_ROW_3, BUTTON_ROW_4};
int               buttonCol[]                = {BUTTON_COL_1, BUTTON_COL_2, BUTTON_COL_3, BUTTON_COL_4};
boolean           buttons[4][4];                                 // stores the button state
boolean           buttonsLast[4][4];                             // button state in the last cycle (used to evaluate changes)
unsigned long     buttonsBounce[4][4];                           // debounce timers for each button
boolean           buttonPressed              = false;            // true while a button is pressed on SEQ and TRACK pages in edit mode
int               buttonPressedNum           = 0;                // the number of the pressed button


// LED Matrix Variables
#define           LEDMATRIX_DEVICE_NUM         1                 // I2C device number
Adafruit_MCP23017 _ledmatrix;
int               ledmatrixBuffer[4][4];                         // store the values that are displayed
int               ledGroundPins[4]           = {LEDMATRIX_GND_1, LEDMATRIX_GND_2, LEDMATRIX_GND_3, LEDMATRIX_GND_4};
int               ledRedPins[4]              = {LEDMATRIX_RED_1, LEDMATRIX_RED_2, LEDMATRIX_RED_3, LEDMATRIX_RED_4};
int               ledGreenPins[4]            = {LEDMATRIX_GRN_1, LEDMATRIX_GRN_2, LEDMATRIX_GRN_3, LEDMATRIX_GRN_4};
int               ledBluePins[4]             = {LEDMATRIX_BLU_1, LEDMATRIX_BLU_2, LEDMATRIX_BLU_3, LEDMATRIX_BLU_4};


// MIDI
int               midiChannel                = 2;                // midi channel number   ******
int               controlChannel             = 13;               // midi channel number    *******
int               sequencerChannel           = 2;                // midi channel number  *******
boolean           secondary                  = false;            // enable secondary midi messages  ***********
int               encodersBanked             = 1;                // *********
int               stepLength                 = 1;
boolean           stackedSteps               = true;
boolean           externalClock              = true;
#define           mtcFPS                       25                // possible FPS: 24, 25, 30 (FPS 29,97 not implemented)   
byte              mtcFPSBits                 = 1<<1;             // use 25 FPS as standard setting
IntervalTimer     mtcTimer;
float             mtcInterval                = (1000000.0 / (mtcFPS * 4)); 
volatile byte     mtcValue;                                      // value of that message
volatile byte     mtcType                    = 0;                // type of the MIDI Time Code Message
volatile byte     mtcF;                                          // counter for the MIDI Time Code Quarter Frame
volatile byte     songH, songM, songS;                           // variables to keep track of the time the current song is playing



// Potentiometer stuff
#define           NOTE                         1                 // note
Potentiometer     *pot1;                                    
int               notePotValue               = 0;
boolean           notePotTakeover            = false;
boolean           notePotChange              = false;
#define           OCTAVE                       2                 // octave
Potentiometer     *pot2;                                    
int               octavePotValue             = 0;
boolean           octavePotTakeover          = false;
boolean           octavePotChange            = false;
#define           VELOCITY                     3                 // velocity
Potentiometer     *pot3;                                    
int               velocityPotValue           = 0;
boolean           velocityPotTakeover        = false;
boolean           velocityPotChange          = false;
#define           PROPABILITY                  4                 // propability
Potentiometer     *pot4;                                   
int               probPotValue               = 0;
boolean           probPotTakeover            = false;
boolean           probPotChange              = false;

boolean           sentPreviewNote            = false;


// Karg: Button            *encBut[8][2];
// Karg: RGLed             *encLed[8][2];
Encoder           encA(ENCA_PIN1, ENCA_PIN2);               // BPM Encoder
Encoder           encB(ENCB_PIN1, ENCB_PIN2);
unsigned long int encNewA[8]                 = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long int encOldA[8]                 = {999, 999, 999, 999, 999, 999, 999, 999};
unsigned long int encNewB[8]                 = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long int encOldB[8]                 = {999, 999, 999, 999, 999, 999, 999, 999};
int               encoderLedValueA[8]        = {0, 0, 0, 0, 0, 0, 0, 0}; //store the dispalyed LED values
int               encoderLedValueB[8]        = {0, 0, 0, 0, 0, 0, 0, 0};
float             encLedOffset               = 4;
int               displayEnc                 = 0;                // 0 none, 1 enc A, 2 enc b, 3 both
unsigned long     displayTimer               = 0;


// step sequencer variables
boolean           sequencerPaused            = true;
int               selectedVoice              = 0;
int               currentStep                = 0;
int               counter                    = 0;

int               clockCounter               = 0;
int               beatCounter                = 0;
byte              CLOCK                      = 0xF8;
byte              START                      = 250;
byte              CONTINUE                   = 251;
byte              STOP                       = 252;

boolean           setupMode                  = false;
int               setupPage                  = 0;

// for midi din out
boolean           midiThrough                = true;
boolean           sequencerMidiOut           = true;
boolean           standaloneSequencer        = true;
float             bpm                        = 120;
boolean           bpmChange                  = false;
int               bpmChangeTime              = 0;

boolean           internalSequencer          = true;
boolean           internalSequencerPlaying   = true;
int               sequencerInterval          = (60000.0 / bpm / 24.0) * 1000;

int               selectedStep               = 0;
boolean           stepEditMode               = false;


//melodic 4 voice sequencer.
int               msSelectedEditPattern      = 0;              
int               msSelectedPattern          = 0;                // the pattern that is currently accessible through the GUI (Pattern # + voices # * 4)
int               msSelectedVoice            = 0;                // current voice #
int               msRootNote[VOICES];
byte              msCurrentStep[VOICES];                         // shows the current step (is influenced by msDirection)
byte              msStepCounter[VOICES];                         // counts the number of steps that have been processed in this pattern to decide when to advance to the next pattern
#define           FORWARD                      0
#define           BACKWARD                     1
#define           PINGPONG                     2
#define           RANDOM                       3
int               msDirection[VOICES];    
int               msStepLength[VOICES];                          // length of eath step: 1: 16th, 2: 8th, 4: quarter, 8: half notes
boolean           msDirAscending[VOICES];
int               msChannel[VOICES];                             // MIDI channel
int               msLength[VOICES];                              // current set length of the patterns
boolean           msLastPlayed[VOICES];
int               msLastPlayedNote[VOICES];
boolean           msMuted[VOICES];
int               msCurrentPattern[VOICES];                      // the currently playing pattern of the currently selected voice
byte              msForceNextPattern[VOICES];                    // next pattern was manually selected through the GUI, override the programmed sequence of patterns
boolean           msVoiceHasPattern[VOICES];                     // has this voice+pattern andy notes set? I guess the array are x/y, meaning voice/pattern
byte              msRepeatCounter[VOICES];                       // how often has it been played?
byte              msRepeatPattern[VOICES][PATTERNS];             // how often should this pattern be repeated
boolean           msHasPattern[VOICES][PATTERNS];
byte              msNextPattern[VOICES][PATTERNS];               // the pattern to play after this one
byte              msStepNote[VOICES][PATTERNS][STEPS];           // only 7 bits are used for MIDI notes, so the highest bit s used for the boolean Legato                            
byte              msStepVelocity[VOICES][PATTERNS][STEPS];       // only 7 bits are used for MIDI velocity, so the highest bit is used to store a boolean variable that indicates if this step is active
byte              msStepChance[VOICES][PATTERNS][STEPS];         // same here
byte              msStepCC[VOICES][PATTERNS][STEPS];
byte              msStepCCValue[VOICES][PATTERNS][STEPS];

boolean           msLastTie[VOICES];

int               msPreviewNote              = 0;
int               msPreviewChannel           = 0;
int               msPreviewTime              = 0;
boolean           msPlayingPreview           = false;


// the midi slide
byte              slVoice[SLIDES];                               // this defines the MIDI channel and the pattern length, highest bit indicates if this slide is set
byte              slStartBeat[SLIDES];                           // i.e. the number of full patterns that have been played already
byte              slStartStep[SLIDES];                           // max 64 steps/patterm , so 6 bits are enough
byte              slStartValue[SLIDES];
byte              slStopBeat[SLIDES];
byte              slStopStep[SLIDES];
byte              slStopValue[SLIDES];
byte              slRemainding[SLIDES];                          // (start-stop)
                                                                 // calculate the step size on the fly by remainding/last-pattern-curretn pattern (include the steps)

IntervalTimer     clockTimer;
IntervalTimer     lcdTimer;


char              scaleNotes[12][12]         = {{0, 2, 3, 5, 7, 9, 11},      // melodic min
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
                                                {0, 2, 4, 5, 7, 9, 10, 11}}; // bebop dominant

int               msScaleLength[12]          = {7, 7, 7, 7, 7, 5, 5, 12, 6, 8, 6, 8};
int               msCurrentScale[VOICES];


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

#define	          DOTPOINT_MASK                80
char              sevencodes[]               = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00, 0x63, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x30, 0x1E, 0x72, 0x38, 0x55, 0x54, 0x5C, 0x73, 0x67, 0x50, 0x2D, 0x78, 0x1C, 0x1C, 0x2A, 0x49, 0x6E, 0x1B};
// original: char sevencodes[]               = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00,0x63,0x77,0x7C,0x39,0x5E,0x79,0x71,0x3D,0x76,0x30,0x1E,0x72,0x38,0x55,0x54,0x5C,0x73,0x67,0x50,0x2D,0x78,0x1C,0x14,0x2A,0x49,0x6E,0x1B};
char              noteStrings[12][3]         = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};
char              scalesStrings[12][5]       = {"melm", "harm", "natm", "majr", "har", "penm", "pent", "chro", "blue", "dimn", "whot", "bebp"};
int               a                          = 0;

int               lcdDigit[]                 = {0, 0, 0, 0};
unsigned long     lcdLastTime                = 0;                // variabel to store lcdDelay time, set to millis() if the timer should be started
unsigned int      lcdDelay                   = 1000;             // time in ms to show page names on lcd
boolean           lcdShowDigit[]             = {false, false, false, false};
boolean           decimal                    = false;

byte              song                       = 0;
byte              patternOffset              = 0;                // pattern offset for tracks page


// Declaration of functions with optional parameters
void lcdPrintStr(char const* str, boolean important = false);



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
boolean noteInScale(int n, int s) {                              // check if a given note interval is in a scale
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





