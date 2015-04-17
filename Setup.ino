void setup() {
  pinMode(0, INPUT_PULLUP); //shift button

  _display.begin(1);  // use default address 1
  _buttonpad.begin(0);      // use default address 0

  initButtonpad();
  initDisplay();      // crlear display
  clearPages();

  // init sequencer
  for (int i=0; i<4; i++){
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
    for(int j=0; j<4; j++){
      msHasPattern[i][j]=false;
      for(int k=0; k<32; k++){
        msStepState[i][j][k]=false;
        msStepNote[i][j][k]=48;
        msStepVelocity[i][j][k]=0;
        msStepChance[i][j][k]=127;
        msStepLegato[i][j][k]=false;
      }
    }
  }
  
  loadSetup();
  
  welcomeAnimation();
  
  for (int i=0; i<8; i++) {
    encBut[i][0]=new Button(11, controlChannel, 0+i*2, false, debug); // encoder buttons
    encBut[i][1]=new Button(2, controlChannel, 1+i*2, false, debug);
    encLed[i][0]=new RGLed(4, 3, controlChannel, 0+i*2, false);//encoder LED
    encLed[i][1]=new RGLed(6, 5, controlChannel, 1+i*2, false);//encoder LED
  }
  
  pot1=new Potentiometer(14, controlChannel, 17, false, debug); // knob on pin 45 (A7)
  pot2=new Potentiometer(15, controlChannel, 18, false, debug); // knob on pin 44 (A6)
  pot3=new Potentiometer(16, controlChannel, 19, false, debug); // knob on pin 45 (A7)
  pot4=new Potentiometer(17, controlChannel, 20, false, debug); // knob on pin 44 (A6)
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  
  lcdPrintStr("seqc");
  
  // setup expansion mux pins
  pinMode(muxAA,OUTPUT);
  pinMode(muxAB,OUTPUT);
  pinMode(muxAC,OUTPUT);
  pinMode(muxAD,OUTPUT);
  // for expansion knobs
  for(int i=0; i<6;i++){
    expPot[i]=new Potentiometer(A6,controlChannel,21+i,false,debug);
  }
  
  //if(stepSequencer)
  setPagePixel(WHITE, 1, 12);
  setPagePixel(WHITE, 1, 13);
  setPagePixel(GREEN, 4, 0);
  setPagePixel(YELLOW, 4, 1);
  setPagePixel(YELLOW, 4, 2);
  setPagePixel(RED, 4, 12);
    
//  MIDI.begin();
  
  usbMIDI.setHandleNoteOff(OnNoteOff); //set event handler for note off
  usbMIDI.setHandleNoteOn(OnNoteOn); //set event handler for note on
  usbMIDI.setHandleControlChange(OnCC); // set event handler for CC
  usbMIDI.setHandleRealTimeSystem(RealTimeSystem); // step sequencer
  usbMIDI.setHandlePitchChange(OnPitchChange);
  
  attachInterrupt(10, readEncoderB, CHANGE);
  attachInterrupt(9, readEncoderB, CHANGE);
  attachInterrupt(8, readEncoderA, CHANGE);
  attachInterrupt(7, readEncoderA, CHANGE);
  
  pinMode(23, INPUT_PULLUP); //shift button
  pinMode(13, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  
  clockTimer.begin(internalSequencerStep,sequencerInterval);
  lcdTimer.begin(sevenSegUpdate,100);
}


