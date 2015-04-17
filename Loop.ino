// #####################################################################################################################
void loop() {
  
  if((bpmChange)&&(!externalClock)){
      //noInterrupts();
      if((millis()-bpmChangeTime)>25){
        clockTimer.end();
        clockTimer.begin(internalSequencerStep,sequencerInterval);
        bpmChange=false;
      }
      //interrupts();
  }
  
  
  // read page shift buttons
  if ( pageShift.update() ) {
    if ( pageShift.read() == LOW) {
      pageSelect=true;
      //Serial.println("page ");
      //displayEnc=0; // stop displaying encoders
      //saveLedState();
    }
    else
    {
      pageSelect=false;
      //displayEnc=0; // stop displaying encoders
      //loadLedState();
    }
  }

  // read edit shift buttons
  if((stepSequencer)&&((page==5)||(page==6)||(page==7))){
    if ( editShift.update() ) {
      if ( editShift.read() == LOW) {
        stepEditMode=!stepEditMode;
        digitalWrite(4,stepEditMode);
      }
    }
  }
  
  if ( editShift2.update() ) {
      if ( editShift2.read() == LOW) {
        stepEditMode=!stepEditMode;
        digitalWrite(4,stepEditMode);
        lcdPrintStr("edit");
      }
    }
	
	if ( seqShift.update() ) {
      if ( seqShift.read() == LOW) {
		if(page==6){
			page=7;
		}
		else{
			page=6;
		}
        lcdPrintStr("seqc");
      }
    }
	
	if ( trackShift.update() ) {
      if ( trackShift.read() == LOW) {
        page=5;
        lcdPrintStr("trac");
		
		for (int i=0; i<4; i++){
		   msChannelHasPattern[i]=false;
		   
		   for(int k=0; k<4; k++){
			msHasPattern[i][k]=false;
			for (int j=0; j<STEPS; j++) {
			  if (msStepState[i][k][j]){
				msHasPattern[i][k]=true;
				msChannelHasPattern[i]=true;
			  }
			  }
			}
		}
		
		for (int i=0; i<4; i++){
       
			for (int j=0; j<4; j++){
			   if(msHasPattern[i][j]){
				 setPagePixel(YELLOW, 5, j*4+i);
			   }
			   else{
				 setPagePixel(0, 5, j*4+i);
			   }
			 }
		   
		   if(msMuted[i])
			 setPagePixel(PINK, 5, msSelectedPattern[i]*4+i);
		   else
			 setPagePixel(GREEN, 5, msSelectedPattern[i]*4+i);
			 
		   if(stepEditMode)                
			 setPagePixel(RED, 5, msSelectedSequence);
		   else
			 setPagePixel(WHITE, 5, msSelectedSequence);
		 }
      }
    }

  if (pageSelect) {
    changePage();
    printBuffer(displayBuffer);
    displayEnc=0; // stop displaying encoders
  }
  else if (displayEnc!=0) {
    if (millis()-displayTimer>200) { // display for a second
      displayEnc=0;
    }
    else {

      switch(displayEnc) {
      case 3:  // show both 
        setLedTo(encoderLedValueA[page*encodersBanked], encoderLedValueB[page*encodersBanked]);
        break;
      case 2:  // show B
        setLedToA(encoderLedValueA[page*encodersBanked]);
        break;
      case 1:  // show A
        setLedToB(encoderLedValueB[page*encodersBanked]);
        break;
      }

      printBuffer(displayBuffer);
      readButtonsGPIO();
    }
  }
  else {
   // display 4 voice patterns
   if ((stepSequencer)&&((page==5))) {
     for (int i=0; i<4; i++){
       
        for (int j=0; j<4; j++){
           if(msHasPattern[i][j]){
             setPagePixel(YELLOW, 5, j*4+i);
           }
           else{
             setPagePixel(0, 5, j*4+i);
           }
         }
       
       if(msMuted[i])
         setPagePixel(PINK, 5, msSelectedPattern[i]*4+i);
       else
         setPagePixel(GREEN, 5, msSelectedPattern[i]*4+i);
         
       if(stepEditMode)                
         setPagePixel(RED, 5, msSelectedSequence);
       else
         setPagePixel(WHITE, 5, msSelectedSequence);
     }
   }
   else if ((stepSequencer)&&((page==3))) {
     setPagePixel(PINK, page, msChannel[msSelectedChannel]-1);
   }
   else if ((stepSequencer)&&((page==2))) {
     setPagePixel(CYAN, page, msCurrentScale[msSelectedChannel]);
   }
   else if ((stepSequencer)&&((page==1))) {
     setPagePixel(YELLOW, page, msRootNote[msSelectedChannel]);
   }
   else if ((stepSequencer)&&((page==0))) {
           if(msStepLength[msSelectedChannel]==1){
           setPagePixel(GREEN, page, 0);
          }else if(msStepLength[msSelectedChannel]==2){
            setPagePixel(GREEN, page, 1);
          }else if(msStepLength[msSelectedChannel]==4){
            setPagePixel(GREEN, page, 2);
          }else if(msStepLength[msSelectedChannel]==8){
            setPagePixel(GREEN, page, 3);
          }
   }
    else if ((stepSequencer)&&((page==6))) {

      for (int i=0; i<STEPS; i++) {
        if((stepEditMode)&&(sellectedStep==i)&&(i<16)){
          setPagePixel(RED, 6, i%16);
        }
        else if ((msStepState[msSelectedChannel][msSelectedSequence/4][i])&&(i<16)) {
          setPagePixel(GREEN, 6, i%16);
        }
        else if ((msStepLegato[msSelectedChannel][msSelectedSequence/4][i])&&(i<16)){
            setPagePixel(YELLOW, 6, i%16);
        }
        else if (i<16) {
          setPagePixel(0, 6, i%16);
        }
      }

      if (msCurrentStep[msSelectedChannel]<16) {
        setPagePixel(BLUE, 6, msCurrentStep[msSelectedChannel]%msLength[msSelectedChannel]%16); //currentStep%midiSeqOutLength[selectedChannel]%16
      }
    }
    else if ((stepSequencer)&&((page==7))) {
      for (int i=0; i<STEPS; i++) {
        if((stepEditMode)&&(sellectedStep==i)&&(i>15)){
          setPagePixel(RED, 7, i%16);
        }      
        else if ((msStepState[msSelectedChannel][msSelectedSequence/4][i])&&(i>15)) {
          setPagePixel(GREEN, 7, i%16);
        }
        else if ((msStepLegato[msSelectedChannel][msSelectedSequence/4][i])&&(i>15)){
            setPagePixel(YELLOW, 7, i%16);
        }
        else if (i>15) {
          setPagePixel(0, 7, i%16);
        }
      }
      if (msCurrentStep[msSelectedChannel]>15) {
        setPagePixel(BLUE, 7, msCurrentStep[msSelectedChannel]%16); //currentStep%midiSeqOutLength[selectedChannel]%16
      }
    }

    printBuffer(displayPages[page]);
    readButtonsGPIO();
  }

  //pot1->readAvr();
  //pot2->readAvr();
  //pot3->readAvr();
  //pot4->readAvr();
  
  readEditPots();
  
  // check preview notes
  if(msPlayingPreview){
    if((millis()-msPreviewTime)>50){
      // (Karg) MIDI.sendNoteOff(msPreviewNote, 0, msPreviewChannel);
      usbMIDI.sendNoteOff(msPreviewNote, 0, msPreviewChannel);
      msPlayingPreview=false;
    }
  }
  //
  
  if(extended){
    // read expansion pots
    for(int i=0; i<6;i++){
      setMux(i);
      //delay(1);
      expPot[i]->readAvr();
    }
  }

  encBut[page*encodersBanked][0]->read();
  encBut[page*encodersBanked][1]->read();

  //encLed[page*encodersBanked][0]->set();
  //encLed[page*encodersBanked][1]->set();

  //readEncoders();

  usbMIDI.read(); // read all the incoming midi messages
}

