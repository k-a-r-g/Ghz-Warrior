// Functions:
//
// void Setup()                                        - Setup routines
// 
// #####################################################################################################################
void setup() {
  initSd();                                    // initialize SD card
  if (sdPresent) loadSetupSd();
  else loadSetupEe();

  pinMode(LCD_LATCH_PIN, OUTPUT);              // initialoze lcd display
  pinMode(LCD_CLOCK_PIN, OUTPUT);
  pinMode(LCD_DATA_PIN, OUTPUT);

  char versionStr[4];
  memcpy(versionStr, versionNum, 3);
  if (sdPresent)  versionStr[3] = 's';
  else versionStr[3] = 'e';
  lcdPrintStr(versionStr);
  lcdTimer.begin(sevenSegUpdate,100);

  pinMode(SHIFT_BUTTON_PIN, INPUT_PULLUP); 
  pinMode(TRACKS_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SEQ_BUTTON_PIN, INPUT_PULLUP);
  pinMode(EDIT_BUTTON_PIN, INPUT_PULLUP);   

  initButtonpad();
  initLEDmatrix();                          // clear display
  clearPages();

  // init sequencer
  for (int i=0; i<VOICES; i++) {            // initialize all voices...
    msCurrentStep[i]=0;
    msDirection[i]=0;
    msRootNote[i]=0;
    msStepLength[i]=1;
    msLastTie[i]=false;
    msDirAscending[i]=false;
    msChannelHasPattern[i]=false;
    msChannel[i]=sequencerChannel+1+i;
    msLength[i]=16;
    msCurrentScale[i]=7;
    msSelectedPattern[i]=0;
    msLastPlayed[i]=false;
    msLastPlayedNote[i]=0;
    msMuted[i]=false;
    for(int j=0; j<PATTERNS; j++){          // ...initialize all patterns in these voice...
      msHasPattern[i][j]=false;
      for(int k=0; k<STEPS; k++){           // ...and each step of these patterns
        msStepState[i][j][k]=false;
        msStepNote[i][j][k]=48;
        msStepVelocity[i][j][k]=0;
        msStepChance[i][j][k]=127;
        msStepLegato[i][j][k]=false;
      }
    }
  }  
    
/* Karg
  for (int i=0; i<8; i++) {
    encBut[i][0]=new Button(16, controlChannel, 0+i*2, false); // encoder buttons
    encBut[i][1]=new Button(2, controlChannel, 1+i*2, false);
    encLed[i][0]=new RGLed(4, 3, controlChannel, 0+i*2, false);//encoder LED
    encLed[i][1]=new RGLed(6, 5, controlChannel, 1+i*2, false);//encoder LED
  } Karg */
  
  pot1=new Potentiometer(OCT_POT_PIN, controlChannel, 17, false, false);
  pot2=new Potentiometer(NOTE_POT_PIN, controlChannel, 18, false, false);
  pot3=new Potentiometer(VEL_POT_PIN, controlChannel, 19, false, false);
  pot4=new Potentiometer(CHANCE_POT_PIN, controlChannel, 20, false, false);
   
  setPagePixel(WHITE, 1, 12);
  setPagePixel(WHITE, 1, 13);
  setPagePixel(GREEN, 4, 0);
  setPagePixel(YELLOW, 4, 1);
  setPagePixel(YELLOW, 4, 2);
  setPagePixel(RED, 4, 12);
    
  MIDI.begin();
  usbMIDI.setHandleNoteOff(OnNoteOff); //set event handler for note off
  usbMIDI.setHandleNoteOn(OnNoteOn); //set event handler for note on
  usbMIDI.setHandleControlChange(OnCC); // set event handler for CC
  usbMIDI.setHandleRealTimeSystem(RealTimeSystem); // step sequencer
  usbMIDI.setHandlePitchChange(OnPitchChange);
  
  attachInterrupt(ENCA_PIN1, readEncoderA, CHANGE);
  attachInterrupt(ENCA_PIN2, readEncoderA, CHANGE);
  attachInterrupt(ENCB_PIN1, readEncoderB, CHANGE);
  attachInterrupt(ENCB_PIN2, readEncoderB, CHANGE);
    
  clockTimer.begin(internalSequencerStep,sequencerInterval);

  welcomeAnimation();

  lcdPrintStr("seqc");
}


