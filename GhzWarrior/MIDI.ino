// Functions:
//
// void OnNoteOn(byte channel, byte note, byte velocity) 
// void OnNoteOff(byte channel, byte note, byte velocity)   
// void OnPitchChange(byte channel, int pitch)   
// void OnCC(byte channel, byte control, byte value)
// void midiNoteOnOff(boolean s, int n)                - ...
// void midiSendMTC()
// void realTimeSystem(byte realtimebyte)    
//
// #####################################################################################################################
// event handlers
void OnNoteOn(byte channel, byte note, byte velocity) {
  // add all your output component sets that will trigger with note ons
  if(encodersBanked==1){
    for (int i=0; i<8;i++) {
// Karg:      encLed[i][0]->setOnSilent(channel, note, velocity);
// Karg:      encLed[i][1]->setOnSilent(channel, note, velocity);
    }
  }else
  {
// Karg:   encLed[0][0]->setOnSilent(channel, note, velocity);
// Karg:   encLed[0][1]->setOnSilent(channel, note, velocity);
  }

  if (channel==midiChannel) {

    if ((note<=127)&&(note>=0))
    {

      int ld=note%16; // find the led
      int pg=note/16; // find the page

      if (velocity!=0) {
        if(velocity==127){
          velocity=pageColor[pg]-1;
        }
        else if ((velocity==1)||(velocity==126)) {
          velocity=GREEN-1;
        }
        else if ((velocity==2)||(velocity==125)) {
          velocity=YELLOW-1;
        }
        else if ((velocity==3)||(velocity==124)) {
          velocity=RED-1;
        }
        setPagePixel((velocity%7)+1, pg, ld);
        //setPagePixel(BLUE,pg,ld);
      }
      else
        setPagePixel(OFF, pg, ld);
    }
  }
  
  if(midiThrough){
    if((channel!=controlChannel)&&(channel!=sequencerChannel)&&(channel!=midiChannel)) MIDI.sendNoteOn(note,velocity,channel);
  }
}

// #####################################################################################################################
void OnNoteOff(byte channel, byte note, byte velocity) {
  // add all your output component sets that will trigger with note ons
  if(encodersBanked==1){
    for (int i=0; i<8;i++) {
// Karg:      encLed[i][0]->setOff(channel, note, velocity);
// Karg:      encLed[i][1]->setOff(channel, note, velocity);
    }
  }else{
// Karg:    encLed[0][0]->setOff(channel, note, velocity);
// Karg:    encLed[0][1]->setOff(channel, note, velocity);
  }

  if (channel==midiChannel) {
    if ((note<=127)&&(note>=0))
    {
      int ld=note%16; // find the led
      int pg=note/16; // find the page
      setPagePixel(OFF, pg, ld);
    }
  }
  
  if(midiThrough){
    if((channel!=controlChannel)&&(channel!=sequencerChannel)&&(channel!=midiChannel)) MIDI.sendNoteOff(note,velocity,channel);
  }
}

// #####################################################################################################################
void OnPitchChange(byte channel, int pitch) {
  if(midiThrough){
    if((channel!=controlChannel)&&(channel!=sequencerChannel)&&(channel!=midiChannel)) MIDI.sendPitchBend (pitch, channel);
  }
}


//######################################################################################
void OnCC(byte channel, byte control, byte value) {
  // add all your output component sets that will trigger with cc
  //led.setOn(channel,control,value);
  if (channel==controlChannel) {
    if ((control>=0)&&(control<16)) {// its in range
      int enc=control%2; //find the encoder
      int pg=control/2; // find the bank


      if (enc==0) { // its first encoder
        encoderLedValueA[pg*encodersBanked]=value;
        encoderLedValueA[pg*encodersBanked]=constrain(encoderLedValueA[pg*encodersBanked], 0, 127);

        if (pg*encodersBanked==page*encodersBanked) { // if on the same page display
          // set the display encoder flag
          if ((displayEnc==1)||(displayEnc==3)) // already displaying encoder
            displayEnc=3; // we want to display both encoders
          else
            displayEnc=2; // just display one encoder
          displayTimer=millis(); // set the time
        }
      }
      if (enc==1) { // its second encoder

        encoderLedValueB[pg*encodersBanked]=value;
        encoderLedValueB[pg*encodersBanked]=constrain(encoderLedValueB[pg*encodersBanked], 0, 127);

        if (pg*encodersBanked==page*encodersBanked) { // if on the same page display
          // set the display encoder flag
          if ((displayEnc==2)||(displayEnc==3)) // already displaying encoder
            displayEnc=3; // we want to display both encoders
          else
            displayEnc=1; // just display one encoder
          displayTimer=millis(); // set the time
        }
      }
    }
  }
  if(midiThrough){
    if((channel!=controlChannel)&&(channel!=sequencerChannel)&&(channel!=midiChannel)) MIDI.sendControlChange(control,value,channel);
  }
}

// #####################################################################################################################
// function to handle note outgoing messages
void midiNoteOnOff(boolean s, int n) {
  if (s) {
    MIDI.sendNoteOn(n, 127, midiChannel);
    usbMIDI.sendNoteOn(n, 127, midiChannel);
    if(secondary) usbMIDI.sendControlChange(n, 127, midiChannel);
  }
  else {
    MIDI.sendNoteOff(n, 0, midiChannel);
    usbMIDI.sendNoteOff(n, 0, midiChannel);
    if(secondary) usbMIDI.sendControlChange(n, 0, midiChannel);
  }
}

// ####################################################################################################################
void midiSendMTC() {
  switch(mtcType){
    case 0: 
      mtcValue=mtcF&0xF;         
      break;
    case 1: 
      mtcValue=(mtcF&0xF0)/16;   
      break;
    case 2:
      mtcValue=songS&0xF;         
      break;
    case 3: 
      mtcValue=(songS&0xF0)/16;   
      break;
    case 4:
      mtcValue=songM&0xF;         
      break;
    case 5:
      mtcValue=(songM&0xF0)/16;   
      break;
    case 6: 
      mtcValue=songH&0xF;         
      break;
    case 7: 
      mtcValue=((songH&0xF0)/16)+mtcFPSBits;  
      break;
  }
  
  MIDI.sendTimeCodeQuarterFrame(mtcType, mtcValue);
  usbMIDI.sendTimeCodeQuarterFrame(mtcType, mtcValue);

  if (++mtcType>7){
    mtcType=0;
    mtcF+=2;
    if (mtcF>mtcFPS-1){
      songS++;
      mtcF-=mtcFPS;
    }
    if (songS>59){
      songM++;
      songS-=60;
    }
    if (songM>59){
      songH++;
      songM-=60;
    }
  }
}


// ####################################################################################################################
// STATUS       HEX    DEC    DESCRIPTION
// 11111000     F8     248    Timing Clock
// 11111001     F9     249    Undefined
// 11111010     FA     250    Start
// 11111011     FB     251    Continue
// 11111100     FC     252    Stop
// 11111101     FD     253    Undefined
// 11111110     FE     254    Active Sensing
// 11111111     FF     255    System Reset
//
void RealTimeSystem(byte realtimebyte) { 

// to do, instead of running the whole sequencer
// just set the internal sequencer bpm based on the incoming clock
  
  if (realtimebyte == CLOCK) {                                       // Clock
    if(!sequencerPaused){
      MIDI.sendRealTime(Clock);
      //usbMIDI.sendRealTime();
		
      clockCounter++;
      executeStep();
      if (clockCounter == STEPS*6) {                 // one MIDI beat is every 6 clocks
        clockCounter = 0;                            // why 6?
        beatCounter++;
        lcdPrintInt(beatCounter+1);
      }
    }
  } 

  if (realtimebyte == START || realtimebyte == CONTINUE) {
    if (realtimebyte == START) clockCounter = 0;
    externalClock=true;
    clockTimer.end();
	
    sequencerPaused=false;
  }

  if (realtimebyte == STOP) {
    lcdPrintStr("stop", true);
    sequencerPaused=true;
    externalClock=false;
    for(int v=0; v<VOICES; v++){
      msCurrentStep[v]=0;
      MIDI.sendControlChange(123,127,msChannel[v]);      // turn all notes off on that channel
      usbMIDI.sendControlChange(123,127,msChannel[v]);
    }
  }
  
  if(realtimebyte == START){
    lcdPrintStr("play", true);
     beatCounter=0;
    for (int v=0;v<VOICES;v++){
      msCurrentPattern[v]=0;
      msCurrentStep[v]=0;
      msStepCounter[v]=0;  
      msRepeatCounter[v]=0;
      checkStep(v);
    }


    // MIDI.sendRealTime(Stop);
    // MIDI.sendSongPosition(0);
    // MIDI.sendRealTime(SystemReset);
    MIDI.sendRealTime(Start); 
  }
  if(realtimebyte == CONTINUE){
    lcdPrintStr("cntn", true);
    MIDI.sendSongPosition(0);
    MIDI.sendRealTime(Continue); 
  }
}

