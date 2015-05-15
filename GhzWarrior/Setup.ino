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

  pinMode(SHIFT_BUTTON_PIN, INPUT_PULLUP);     // intitalize the buttons
  pinMode(TRACKS_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SEQ_BUTTON_PIN, INPUT_PULLUP);
  pinMode(EDIT_BUTTON_PIN, INPUT_PULLUP);   

  pot1=new Potentiometer(NOTE_POT_PIN, controlChannel, 18, false, false); // PIN, midi channel, cc number, secondary super know, debug
  pot2=new Potentiometer(OCT_POT_PIN, controlChannel, 17, false, false); 
  pot3=new Potentiometer(VEL_POT_PIN, controlChannel, 19, false, false);
  pot4=new Potentiometer(CHANCE_POT_PIN, controlChannel, 20, false, false);
  readEditPots();    

  initButtonpad();
  initLEDmatrix();                          // clear display
  clearPages();
  
  /* Karg
  for (int i=0; i<8; i++) {
    encBut[i][0]=new Button(16, controlChannel, 0+i*2, false); // encoder buttons
    encBut[i][1]=new Button(2, controlChannel, 1+i*2, false);
    encLed[i][0]=new RGLed(4, 3, controlChannel, 0+i*2, false);//encoder LED
    encLed[i][1]=new RGLed(6, 5, controlChannel, 1+i*2, false);//encoder LED
  } Karg */


  // init sequencer
  for (int v=0; v<VOICES; v++) {            // initialize all voices...
    msCurrentStep[v]=0;
    msStepCounter[v]=0; 
    msDirection[v]=FORWARD;
    msRootNote[v]=0;
    msStepLength[v]=1;
    msLastTie[v]=false;
    msDirAscending[v]=false;
    msVoiceHasPattern[v]=false;
    msForceNextPattern[v]=255;
    msRepeatCounter[v]=0;
    msChannel[v]=sequencerChannel+1+v;
    msLength[v]=64;
    msCurrentScale[v]=7;
    msCurrentPattern[v]=0;
    msLastPlayed[v]=false;
    msLastPlayedNote[v]=0;
    msMuted[v]=false;
    for(int p=0; p<PATTERNS; p++){          // ...initialize all patterns in these voice...
      msHasPattern[v][p]=false;
      msRepeatPattern[v][p]=0;
      msNextPattern[v][p]=p;
      for(int s=0; s<STEPS; s++){           // ...and each step of these patterns
        msStepNote[v][p][s]=48;
        msStepVelocity[v][p][s]=0;
        msStepChance[v][p][s]=127;
        msStepCC[v][p][s]=0;
        msStepCCValue[v][p][s]=0;
      }
    }
  }  
  
  for (int s=0; s<SLIDES; s++){             // initialize MIDI slides
    slVoice[s]=0; 
    slStartBeat[s]=0;
    slStartStep[s]=0;
    slStartValue[s]=0;
    slStopBeat[s]=0;
    slStopStep[s]=0;
    slStopValue[s]=0;
    slRemainding[s]=0;                    
  }
             
  MIDI.begin();
  usbMIDI.setHandleNoteOff(OnNoteOff);      //set event handler for note off
  usbMIDI.setHandleNoteOn(OnNoteOn);        //set event handler for note on
  usbMIDI.setHandleControlChange(OnCC);     // set event handler for CC
  usbMIDI.setHandleRealTimeSystem(RealTimeSystem); // step sequencer
  usbMIDI.setHandlePitchChange(OnPitchChange);
  
  attachInterrupt(ENCA_PIN1, readEncoderA, CHANGE);
  attachInterrupt(ENCA_PIN2, readEncoderA, CHANGE);
  attachInterrupt(ENCB_PIN1, readEncoderB, CHANGE);
  attachInterrupt(ENCB_PIN2, readEncoderB, CHANGE);
    
  //if (!externalClock) 
  clockTimer.begin(internalSequencerStep,sequencerInterval);

  welcomeAnimation();

  lcdPrintStr("sen1", true);
}


