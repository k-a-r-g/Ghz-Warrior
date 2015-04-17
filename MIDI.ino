
// #####################################################################################################################
// function to handle noteon outgoing messages
void midiNoteOnOff(boolean s, int n) {

  if (s) {
    if (debug) {//debbuging enabled
      Serial.print("Button ");
      Serial.print(n);
      Serial.println(" pressed.");
    }
    else {
      usbMIDI.sendNoteOn(n, 127, midiChannel);
      if(secondary)
        usbMIDI.sendControlChange(n, 127, midiChannel);
    }
  }
  else {
    if (debug) {//debbuging enabled
      Serial.print("Button ");
      Serial.print(n);
      Serial.println(" released.");
    }
    else {
      usbMIDI.sendNoteOff(n, 0, midiChannel);
      if(secondary)
        usbMIDI.sendControlChange(n, 0, midiChannel);
    }
  }
}

// #####################################################################################################################
// event handlers
void OnNoteOn(byte channel, byte note, byte velocity) {
  // add all your output component sets that will trigger with note ons
  if(encodersBanked==1){
    for (int i=0; i<8;i++) {
      encLed[i][0]->setOnSilent(channel, note, velocity);
      encLed[i][1]->setOnSilent(channel, note, velocity);
    }
  }else
  {
   encLed[0][0]->setOnSilent(channel, note, velocity);
   encLed[0][1]->setOnSilent(channel, note, velocity);
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
        setPagePixel(0, pg, ld);
    }
  }
  
  if(midiThrough){
    if((channel!=controlChannel)&&(channel!=sequencerChannel)&&(channel!=midiChannel)){
      // (Karg) MIDI.sendNoteOn(note,velocity,channel);
    }
  }
}

// #####################################################################################################################
void OnNoteOff(byte channel, byte note, byte velocity) {
  // add all your output component sets that will trigger with note ons
  if(encodersBanked==1){
    for (int i=0; i<8;i++) {
      encLed[i][0]->setOff(channel, note, velocity);
      encLed[i][1]->setOff(channel, note, velocity);
    }
  }else{
    encLed[0][0]->setOff(channel, note, velocity);
    encLed[0][1]->setOff(channel, note, velocity);
  }

  if (channel==midiChannel) {
    if ((note<=127)&&(note>=0))
    {
      int ld=note%16; // find the led
      int pg=note/16; // find the page
      setPagePixel(0, pg, ld);
    }
  }
  
  if(midiThrough){
    if((channel!=controlChannel)&&(channel!=sequencerChannel)&&(channel!=midiChannel)){
      // (Karg) MIDI.sendNoteOff(note,velocity,channel);
    }
  }
}

// #####################################################################################################################
void OnPitchChange(byte channel, int pitch) {
  if(midiThrough){
    if((channel!=controlChannel)&&(channel!=sequencerChannel)&&(channel!=midiChannel)){
      // (Karg) MIDI.sendPitchBend (pitch, channel);
    }
  }
}

// ####################################################################################################################
void RealTimeSystem(byte realtimebyte) { 

// to do, instead of running the whole sequencer
// just set the internal sequencer bpm based on the incoming clock
  
  if (realtimebyte == 248) {
	if(!sequencerPaused){
		// (Karg) MIDI.sendRealTime(Clock);
		//usbMIDI.sendRealTimeClock();
		
		clockCounter++; 

		  for(int i=0; i<4;i++){
			if (clockCounter%(6*msStepLength[i]) == 0) { //6
			if(msDirection[i]==0){  //forward
			  msCurrentStep[i]++;
			  if (msCurrentStep[i]>=msLength[i])
				msCurrentStep[i]=0;
			}
			if(msDirection[i]==1){  //backwards
			  msCurrentStep[i]--;
			  if (msCurrentStep[i]<0)
				msCurrentStep[i]=msLength[i]-1;
			}
			if(msDirection[i]==2){ // ping pong
			  if(msDirAscending[i]){  //forward
				msCurrentStep[i]++;
				if (msCurrentStep[i]>=msLength[i]){
				  msDirAscending[i]=false;
				  msCurrentStep[i]-=2;
				}
			  }else{  //backwards
				msCurrentStep[i]--;
				if (msCurrentStep[i]<0){
				  msDirAscending[i]=true;
				  msCurrentStep[i]+=2;
				}
			  }
			}
			if(msDirection[i]==3){  //random
			  msCurrentStep[i]=random(0,msLength[i]);
			}
		  checkStep(i);
		  }
		}

		if (clockCounter == 48) { //24
		  clockCounter = 0;
		}
	  }
  } 

  if (realtimebyte == START || realtimebyte == CONTINUE) {
	externalClock=true;
	clockTimer.end();
	
    sequencerPaused=false;
	for(int i=0; i<4; i++){
	  msCurrentStep[i]=0;
	  checkStep(i);
	}
  }

  if (realtimebyte == STOP) {
    sequencerPaused=true;
	externalClock=false;
	for(int i=0; i<4; i++){
	  msCurrentStep[i]=0;
	}
  }
  
   if(realtimebyte == START){
       //    MIDI.sendRealTime(Stop);
       //MIDI.sendSongPosition(0);
       //    MIDI.sendRealTime(SystemReset);
       // (Karg) MIDI.sendRealTime(Start); 
     }
   if(realtimebyte == CONTINUE){
       // (Karg) MIDI.sendSongPosition(0);
       // (Karg) MIDI.sendRealTime(Continue); 
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
    if((channel!=controlChannel)&&(channel!=sequencerChannel)&&(channel!=midiChannel)){
      // (Karg) MIDI.sendControlChange(control,value,channel);
    }
  }
}

