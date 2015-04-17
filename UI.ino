// #####################################################################################################################
void changePage() {
  noInterrupts();
  int state;
  int bState;

  //row
  for (int row=0; row<4; row++) {
    _buttonpad.writeGPIO(240-(1<<buttonRow[row]));
    state = _buttonpad.readGPIO();    
    for (int col =0; col<4; col++) {

      bState=state&(1<<col);

      if ((bState==0)&&(bState!=buttons[row][col])&&((millis() - buttonsBounce[row][col]) >bounceDelay)) { //pressed
        buttonsBounce[row][col]=millis();
        int p=(4*row+col)+(col*3-row*3);
        
        if ((stepSequencer)&&(p==15)){ // step sequencer
          sequencerPaused=!sequencerPaused;
          if (!sequencerPaused) {
            // (Karg) MIDI.sendSongPosition(0);
            //usbMIDI.sendSongPosition(0);
            // (Karg) MIDI.sendRealTime(Start);
            //usbMIDI.sendRealTimeStart();
            clockCounter = 0; 
            setPagePixel(0, 6+currentStep/STEPS, currentStep%16);
            currentStep=0; 
            setPagePixel(BLUE, 6+(currentStep/STEPS), currentStep%16);
            checkStep(0);
            checkStep(1);
            checkStep(2);
            checkStep(3);
          }else{
            // (Karg) MIDI.sendSongPosition(0);
            // (Karg) MIDI.sendRealTime(Stop);
            //usbMIDI.sendSongPosition(0);
            //usbMIDI.sendRealTimeStop();
            //for(int i=0; i<4; i++){
			//  msCurrentStep[i]=0;
			//}
          }
        }
          
        if((stepSequencer)&&(p==13)&&(msHasPattern[msSelectedChannel][msSelectedSequence/4])){ // delete sequence
          //for(int j=0; j<4; j++)
            for(int i=0; i<32; i++){
              msStepState[msSelectedChannel][msSelectedSequence/4][i]=false;
              msStepNote[msSelectedChannel][msSelectedSequence/4][i]=48;
              msStepVelocity[msSelectedChannel][msSelectedSequence/4][i]=0;
              msStepChance[msSelectedChannel][msSelectedSequence/4][i]=127;
              msStepLegato[msSelectedChannel][msSelectedSequence/4][i]=false;
            }
            //msLength[msSelectedChannel]=16;
            //msMuted[msSelectedChannel]=false;
          }
        
        if((stepSequencer)&&(p==12)){
			if (!sequencerPaused) {
					// (Karg) MIDI.sendSongPosition(0);
					// (Karg) MIDI.sendRealTime(Start);
				}
				else
				{
					// (Karg) MIDI.sendSongPosition(0);
					// (Karg) MIDI.sendRealTime(Stop);
				}
			  //savePatterns(selectedChannel);
			clockCounter = 0;
			
			for(int i=0; i<4; i++){
			  msCurrentStep[i]=0;
			  checkStep(i);
			}
          //savePatternAnimation();
        }
        
        if((stepSequencer)&&((p>7)&&(p<12))){
          //selectedPattern[selectedChannel]=p-8;
          msDirection[msSelectedChannel]=p-8;
        }
          
        if((stepSequencer)&&(p==14)&&(msChannelHasPattern[msSelectedChannel])){ // mute channel
          msMuted[msSelectedChannel]=!msMuted[msSelectedChannel];
        }
        else if (p<8) { // change page
          page = p;
          //Serial.print(" Page ");
          //Serial.println(p);
		  switch (page){
			case 0:
				lcdPrintStr("dur");
				break;
			case 1:
				lcdPrintStr("tran");
				break;
			case 2:
				lcdPrintStr("scal");
				break;
			case 3:
				lcdPrintStr("chan");
				break;
			case 4:
				lcdPrintStr("song");
				break;
			case 5:
				lcdPrintStr("trac");
				break;
			case 6:
			case 7:
				lcdPrintStr("seqc");
				break;
		  }
        }
        //midiNoteOnOff(true,(4*row+col)+(col*3-row*3));
        //setBufferPixel(random(1,7),col,row);
      }
      buttons[row][col] = bState;
    }
  }
  interrupts();
  clearBuffer();
  
  if (stepSequencer) {
    
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
	
	if(msHasPattern[msSelectedChannel][msSelectedSequence/4])
	      setBufferPixel(RED, 13);
    
    if(msChannelHasPattern[msSelectedChannel]){

    if(msMuted[msSelectedChannel])
        setBufferPixel(YELLOW, 14);
      else
        setBufferPixel(GREEN, 14);
    }
    setBufferPixel(WHITE, 5);

    if (msCurrentStep[msSelectedChannel]<16) {
      setBufferPixel(CYAN, 6);
    }
    else
      setBufferPixel(BLUE, 6);

    if (STEPS==32) {
      if (msCurrentStep[msSelectedChannel]>15)
        setBufferPixel(CYAN, 7);
      else
        setBufferPixel(BLUE, 7);
    }
    
  if (!sequencerPaused)
    setBufferPixel(BLUE, 15);
  else
    setBufferPixel(YELLOW, 15);
    
    //show the sellected pattern direction
    setBufferPixel(PINK, msDirection[msSelectedChannel]+8);
  }

  setBufferPixel(PINK, page);
}



// #####################################################################################################################
void readEncoderB() {
  noInterrupts();
  encNewB[page*encodersBanked] = encB.read();
  //encoderLedValueB[page] = constrain(newPositionB[page],0,127);
  
   if((stepSequencer)&&(stepEditMode)&&((page==6)||(page==7))){
    if (encNewB[page*encodersBanked]/4 != encOldB[page*encodersBanked]/4) {
      
      if (encNewB[page*encodersBanked]/4>encOldB[page*encodersBanked]/4) {
        msLength[msSelectedChannel]++;
      }else if (encNewB[page*encodersBanked]/4<encOldB[page*encodersBanked]/4){
        msLength[msSelectedChannel]--;
      }
      msLength[msSelectedChannel]=constrain(msLength[msSelectedChannel],2,STEPS);
      
      lcdPrintInt(msLength[msSelectedChannel]);
      
      // display step size
      if((msLength[msSelectedChannel]>16)&&(page==6))
       encoderLedValueB[page*encodersBanked] = 127;
      else if((msLength[msSelectedChannel]>16)&&(page==7))
        encoderLedValueB[page*encodersBanked] = (msLength[msSelectedChannel]-16)*8; //map(msLength[msSelectedChannel]-16, -1, 15,0,127);
      else if((msLength[msSelectedChannel]<17)&&(page==7))
        encoderLedValueB[page*encodersBanked] = 0;
      else if((msLength[msSelectedChannel]<17)&&(page==6))
        encoderLedValueB[page*encodersBanked] = msLength[msSelectedChannel]*8;

          // set the display encoder flag
          if ((displayEnc==2)||(displayEnc==3)) // already displaying encoder
            displayEnc=3; // we want to display both encoders
          else
            displayEnc=1; // just display one encoder
    }
  }

  if (encNewB[page*encodersBanked] != encOldB[page*encodersBanked]) {

    if (encNewB[page*encodersBanked]>encOldB[page*encodersBanked]) {
      if (debug) {
        Serial.print(" encoder ");
        Serial.print(1);
        Serial.println(" >> ");
      }
      else {   
        if (abletonEncoder) {
          usbMIDI.sendControlChange(1+page*2*encodersBanked, 70, controlChannel);
          encoderLedValueB[page*encodersBanked]+=encLedOffset;
          encoderLedValueB[page*encodersBanked] = constrain(encoderLedValueB[page*encodersBanked], 0, 127);

          // set the display encoder flag
          if ((displayEnc==2)||(displayEnc==3)) // already displaying encoder
            displayEnc=3; // we want to display both encoders
          else
            displayEnc=1; // just display one encoder
        }
        else     
          usbMIDI.sendControlChange(1+page*2*encodersBanked, 1, controlChannel);
      }
    }
    else if (encNewB[page*encodersBanked]<encOldB[page*encodersBanked])
    {
      if (debug) {
        Serial.print(" encoder ");
        Serial.print(1);
        Serial.println(" << ");
      }
      else {//for ableton
        if (abletonEncoder) {
          usbMIDI.sendControlChange(1+page*2*encodersBanked, 58, controlChannel);
          encoderLedValueB[page*encodersBanked]-=encLedOffset;
          encoderLedValueB[page*encodersBanked] = constrain(encoderLedValueB[page*encodersBanked], 0, 127);

          // set the display encoder flag
          if ((displayEnc==2)||(displayEnc==3)) // already displaying encoder
            displayEnc=3; // we want to display both encoders
          else
            displayEnc=1; // just display one encoder
        }
        else 
          usbMIDI.sendControlChange(1+page*2*encodersBanked, 127, controlChannel);
      }
    }

    encOldB[page*encodersBanked] = encNewB[page*encodersBanked];
    displayTimer=millis(); // set the time
  }
  interrupts();
}

void readEncoderA() {
  noInterrupts();
  encNewA[page*encodersBanked] = encA.read();
  
  if((stepSequencer)&&(stepEditMode)&&((page==6)||(page==7))){
    if (encNewA[page*encodersBanked] != encOldA[page*encodersBanked]) {
      
      if (encNewA[page*encodersBanked]>encOldA[page*encodersBanked]) {
        bpm+=0.1;
      }else if (encNewA[page*encodersBanked]<encOldA[page*encodersBanked]){
        bpm-=0.1;
      }
      bpm=constrain(bpm,40,240);
      
      sequencerInterval= (60000.0/bpm/24.0)*1000;
      bpmChange=true;
      bpmChangeTime=millis();
      
      lcdPrintFloat(bpm);
      
//      encoderLedValueA[page*encodersBanked] = msStepChance[msSelectedChannel][msSelectedSequence/4][sellectedStep];
//
//          // set the display encoder flag
//          if ((displayEnc==1)||(displayEnc==3)) // already displaying encoder
//            displayEnc=3; // we want to display both encoders
//          else
//            displayEnc=2; // just display one encoder
    }
  }

  if (encNewA[page*encodersBanked] != encOldA[page*encodersBanked]) {

    if (encNewA[page*encodersBanked]>encOldA[page*encodersBanked]) {
      if (debug) {
        Serial.print(" encoder ");
        Serial.print(1);
        Serial.println(" >> ");
      }
      else {
        if (abletonEncoder) {
          usbMIDI.sendControlChange(page*2*encodersBanked, 70, controlChannel);

          encoderLedValueA[page*encodersBanked]+=encLedOffset;
          encoderLedValueA[page*encodersBanked] = constrain(encoderLedValueA[page*encodersBanked], 0, 127);
          // set the display encoder flag
          if ((displayEnc==1)||(displayEnc==3)) // already displaying encoder
            displayEnc=3; // we want to display both encoders
          else
            displayEnc=2; // just display one encoder
        }
        else    
          usbMIDI.sendControlChange(page*2*encodersBanked, 1, controlChannel);
      }
    }
    else if (encNewA[page*encodersBanked]<encOldA[page*encodersBanked])
    {
      if (debug) {
        Serial.print(" encoder ");
        Serial.print(1);
        Serial.println(" << ");
      }
      else {//for ableton
        if (abletonEncoder) {
          usbMIDI.sendControlChange(page*2*encodersBanked, 58, controlChannel);

          encoderLedValueA[page*encodersBanked]-=encLedOffset;
          encoderLedValueA[page*encodersBanked] = constrain(encoderLedValueA[page*encodersBanked], 0, 127);
          // set the display encoder flag
          if ((displayEnc==1)||(displayEnc==3)) // already displaying encoder
            displayEnc=3; // we want to display both encoders
          else
            displayEnc=2; // just display one encoder
        }
        else 
          usbMIDI.sendControlChange(page*2*encodersBanked, 127, controlChannel);
      }
    }

    encOldA[page*encodersBanked] = encNewA[page*encodersBanked];
    displayTimer=millis(); // set the time
  }
  interrupts();
}



// #####################################################################################################################
void readButtonsGPIO() {
  noInterrupts();
  int state;
  int bState;

  //row
  for (int row=0; row<4; row++) {
    _buttonpad.writeGPIO(240-(1<<buttonRow[row]));
    state = _buttonpad.readGPIO();    
    for (int col =0; col<4; col++) {

      bState=state&(1<<col);
      
      if(bState!=buttons[row][col]){
        
        if ((millis() - buttonsBounce[row][col])>bounceDelay){

          if (bState==0) {
            buttonsBounce[row][col]=millis();
            displayEnc=0; // stop displaying encoders
            //Serial.print((4*row+col)+(col*3-row*3));
            //Serial.println(" pressed");
            if ((stepSequencer)&&(page==5)) // sellect a voice
            {
              setPagePixel(0, page, msSelectedSequence);
              setPagePixel(0, page, msSelectedPattern[row]*4+row);
              
              setPagePixel(0, 3, msChannel[msSelectedChannel]-1);
              setPagePixel(0, 2, msCurrentScale[msSelectedChannel]);
              setPagePixel(0, 1, msRootNote[msSelectedChannel]);
              
              if(msStepLength[msSelectedChannel]==1){
               setPagePixel(0, 0, 0);
              }else if(msStepLength[msSelectedChannel]==2){
                setPagePixel(0, 0, 1);
              }else if(msStepLength[msSelectedChannel]==4){
                setPagePixel(0, 0, 2);
              }else if(msStepLength[msSelectedChannel]==8){
                setPagePixel(0, 0, 3);
              }
              
              if(stepEditMode){
                msSelectedSequence=4*col+row;
                msSelectedChannel=msSelectedSequence%4;
                
                if(!stepPressed){
                    stepPressed=true;
                    stepPressedNum=msSelectedSequence;
                  }else{
                    for(int i=0; i<32; i++){
                      msStepState[msSelectedSequence%4][msSelectedSequence/4][i]=msStepState[stepPressedNum%4][stepPressedNum/4][i];
                      msStepNote[msSelectedSequence%4][msSelectedSequence/4][i]=msStepNote[stepPressedNum%4][stepPressedNum/4][i];
                      msStepVelocity[msSelectedSequence%4][msSelectedSequence/4][i]=msStepVelocity[stepPressedNum%4][stepPressedNum/4][i];
                      msStepChance[msSelectedSequence%4][msSelectedSequence/4][i]=msStepChance[stepPressedNum%4][stepPressedNum/4][i];
                      msStepLegato[msSelectedSequence%4][msSelectedSequence/4][i]=msStepLegato[stepPressedNum%4][stepPressedNum/4][i];
                    }
                    msHasPattern[msSelectedSequence%4][msSelectedSequence/4]=msHasPattern[stepPressedNum%4][stepPressedNum/4];
                    msChannelHasPattern[msSelectedSequence%4]=msChannelHasPattern[stepPressedNum%4];
                  }
              }
              else{
                msSelectedPattern[row]=col;
                msSelectedSequence=4*col+row;
                msSelectedChannel=msSelectedSequence%4;
              }
              //setPagePixel(WHITE, page, msSelectedPattern[row]*4+row);
            }
            else if ((stepSequencer)&&(page==3)) // sellect sequence channel
            {
              setPagePixel(0, page, msChannel[msSelectedChannel]-1);
              msChannel[msSelectedChannel]=4*col+row+1;
              setPagePixel(PINK, page, msChannel[msSelectedChannel]-1);
              lcdPrintInt(msChannel[msSelectedChannel]);
            }
			else if(page==4)// song save page
			{
				int p=4*col+row;
				
				setPagePixel(YELLOW, page, song);
				//turn off settings leds
				setPagePixel(0, 3, msChannel[msSelectedChannel]-1);
				setPagePixel(0, 2, msCurrentScale[msSelectedChannel]);
				setPagePixel(0, 1, msRootNote[msSelectedChannel]);
				
				if(msStepLength[msSelectedChannel]==1){
				   setPagePixel(0, 0, 0);
				  }else if(msStepLength[msSelectedChannel]==2){
					setPagePixel(0, 0, 1);
				  }else if(msStepLength[msSelectedChannel]==4){
					setPagePixel(0, 0, 2);
				  }else if(msStepLength[msSelectedChannel]==8){
					setPagePixel(0, 0, 3);
				  }
				
				switch(p){
					case 12: // save
						savePatterns(song);
						break;
					case 0:
					case 1:
					case 2:
						
						song=p;
						loadPatterns(song);
						//load song;
						break;
				}
				
				setPagePixel(GREEN, page, song);
				
			}
            else if ((stepSequencer)&&(page==2)) // sellect sequence scale
            {
              setPagePixel(0, page, msCurrentScale[msSelectedChannel]);
              if(stepEditMode)
                msCurrentScale[msSelectedChannel]=constrain(4*col+row,0,11);
              else
                for(int i=0;i<4;i++){
                  msCurrentScale[i]=constrain(4*col+row,0,11);
                }
              setPagePixel(CYAN, page, msCurrentScale[msSelectedChannel]);
              if(sequencerPaused){
                for(int i=0; i<msScaleLength[msCurrentScale[msSelectedChannel]]-1; i++){
                  playPreviewNote(msRootNote[msSelectedChannel]+48+scaleNotes[msCurrentScale[msSelectedChannel]][i], 127, msChannel[msSelectedChannel]);
                  delay(110);
                }
                for(int i=msScaleLength[msCurrentScale[msSelectedChannel]]-1; i>=0; i--){
                  playPreviewNote(msRootNote[msSelectedChannel]+48+scaleNotes[msCurrentScale[msSelectedChannel]][i], 127, msChannel[msSelectedChannel]);
                  delay(110);
                }
              }
			  // fix scale notes;
			  if(stepEditMode){//fix only for this track{
				for(int p=0;p<4;p++)
					for(int s=0;s<32;s++){
						while(!noteInScale(msStepNote[msSelectedChannel][p][s],msCurrentScale[msSelectedChannel])){
							msStepNote[msSelectedChannel][p][s]++;
						}
					}
			  }
			  else { // fix for all tracks
				  for(int i=0;i<4;i++){
					  for(int p=0;p<4;p++)
						for(int s=0;s<32;s++){
							while(!noteInScale(msStepNote[i][p][s],msCurrentScale[i])){
								msStepNote[i][p][s]++;
							}
						}
					}
			  }
              lcdPrintStr(scalesStrings[msCurrentScale[msSelectedChannel]]);
            }
            else if ((stepSequencer)&&(page==1)) // sellect sequence root note
            {
				if(col==3){ // change octave
					if(row==0){//down
						for(int p=0;p<4;p++){
							for(int s=0;s<32;s++){
								if((msStepNote[msSelectedChannel][p][s]/12)>0)
									msStepNote[msSelectedChannel][p][s]-=12;
							}
						}
					}
					if(row==1){//up
						for(int p=0;p<4;p++){
							for(int s=0;s<32;s++){
								if((msStepNote[msSelectedChannel][p][s]/12)<9)
									msStepNote[msSelectedChannel][p][s]+=12;
							}
						}
					}
				}
				else{
              setPagePixel(0, page, msRootNote[msSelectedChannel]);
              if(stepEditMode)
                msRootNote[msSelectedChannel]=constrain(4*col+row,0,11);
              else
                for(int i=0; i<4; i++){
                  msRootNote[i]=constrain(4*col+row,0,11);
                }
              setPagePixel(YELLOW, page, msRootNote[msSelectedChannel]);
              if(sequencerPaused){
                playPreviewNote(msRootNote[msSelectedChannel]+48, 127, msChannel[msSelectedChannel]);
              }
              lcdPrintStr(noteStrings[msRootNote[msSelectedChannel]]);
			  }
			  
            }
            else if ((stepSequencer)&&(page==0)) // sellect sequence length
            {
              
              
          if(msStepLength[msSelectedChannel]==1){
           setPagePixel(0, page, 0);
          }else if(msStepLength[msSelectedChannel]==2){
            setPagePixel(0, page, 1);
          }else if(msStepLength[msSelectedChannel]==4){
            setPagePixel(0, page, 2);
          }else if(msStepLength[msSelectedChannel]==8){
            setPagePixel(0, page, 3);
          }
              
           if(4*col+row==0){
            msStepLength[msSelectedChannel]=1;
          }else if(4*col+row==1){
            msStepLength[msSelectedChannel]=2;
          }else if(4*col+row==2){
            msStepLength[msSelectedChannel]=4;
          }else if(4*col+row==3){
            msStepLength[msSelectedChannel]=8;
          }
          
          if(msStepLength[msSelectedChannel]==1){
           setPagePixel(GREEN, page, 0);
           lcdPrintInt(16);
          }else if(msStepLength[msSelectedChannel]==2){
            setPagePixel(GREEN, page, 1);
            lcdPrintInt(8);
          }else if(msStepLength[msSelectedChannel]==4){
            setPagePixel(GREEN, page, 2);
            lcdPrintInt(4);
          }else if(msStepLength[msSelectedChannel]==8){
            setPagePixel(GREEN, page, 3);
            lcdPrintInt(2);
          }
            }
            else if ((stepSequencer)&&(page==5)) // sellect a voice
            {
              //setPagePixel(0, page, selectedChannel);
              //selectedChannel=4*col+row;
              //setPagePixel(WHITE, page, selectedChannel);
            }
            else if ((stepSequencer)&&(page==6)) // enter pattern
            {
              if(!stepEditMode){
                //stepState[selectedPattern[selectedChannel]][selectedChannel][4*col+row]=!stepState[selectedPattern[selectedChannel]][selectedChannel][4*col+row];
                msStepState[msSelectedChannel][msSelectedSequence/4][4*col+row]=!msStepState[msSelectedChannel][msSelectedSequence/4][4*col+row];
				if(msStepState[msSelectedChannel][msSelectedSequence/4][4*col+row]){
					bool velocityChange=false;
					msStepVelocity[msSelectedChannel][msSelectedSequence/4][4*col+row]=pot3->readValueAvr(velocityChange);
					}
				else
					msStepVelocity[msSelectedChannel][msSelectedSequence/4][4*col+row]=0;
              }else{
                if(!stepPressed){
                    stepPressed=true;
                    stepPressedNum=4*col+row;
                  }else{
                    if(4*col+row-stepPressedNum>0){
                      for(int i=stepPressedNum+1; i<=4*col+row; i++)
                        msStepLegato[msSelectedChannel][msSelectedSequence/4][i]=!msStepLegato[msSelectedChannel][msSelectedSequence/4][i];
                    }
                    else{
                      for(int i=4*col+row+1; i<=stepPressedNum; i++)
                        msStepLegato[msSelectedChannel][msSelectedSequence/4][i]=!msStepLegato[msSelectedChannel][msSelectedSequence/4][i];
                    }
                  }
              }
              sellectedStep=4*col+row;
              octaveKnobTakeover=false;
              noteKnobTakeover=false;
              velocityKnobTakeover=false;
              vknob=false;
			  probknob=false;
              lcdPrintStr(noteStrings[(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep]) % 12]);
            }
            else if ((stepSequencer)&&(page==7)&&(STEPS==32)) { // enter pattern second
              if(!stepEditMode){
                //stepState[selectedPattern[selectedChannel]][selectedChannel][4*col+row+16]=!stepState[selectedPattern[selectedChannel]][selectedChannel][4*col+row+16];
                msStepState[msSelectedChannel][msSelectedSequence/4][4*col+row+16]=!msStepState[msSelectedChannel][msSelectedSequence/4][4*col+row+16];
				if(msStepState[msSelectedChannel][msSelectedSequence/4][4*col+row+16]){
					bool velocityChange=false;
					msStepVelocity[msSelectedChannel][msSelectedSequence/4][4*col+row+16]=pot3->readValueAvr(velocityChange);
					}
				else
					msStepVelocity[msSelectedChannel][msSelectedSequence/4][4*col+row+16]=0;
              }else{
                if(!stepPressed){
                    stepPressed=true;
                    stepPressedNum=4*col+row+16;
                  }else{
                    if(4*col+row+16-stepPressedNum>0){
                      for(int i=stepPressedNum+1; i<=4*col+row+16; i++)
                        msStepLegato[msSelectedChannel][msSelectedSequence/4][i]=!msStepLegato[msSelectedChannel][msSelectedSequence/4][i];
                    }
                    else{
                      for(int i=4*col+row+16+1; i<=stepPressedNum; i++)
                        msStepLegato[msSelectedChannel][msSelectedSequence/4][i]=!msStepLegato[msSelectedChannel][msSelectedSequence/4][i];
                    }
                  }
              }
              
              sellectedStep=4*col+row+16;
              octaveKnobTakeover=false;
              noteKnobTakeover=false;
              velocityKnobTakeover=false;
              vknob=false;
			  probknob=false;
              lcdPrintStr(noteStrings[(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep]) % 12]);
            }
            else
              midiNoteOnOff(true, (4*row+col)+(col*3-row*3)+page*16);
            //setBufferPixel(random(1,7),col,row);
          }
          else if (bState!=0) {
            //Serial.print((4*row+col)+(col*3-row*3));
            //Serial.println(" released");
            if((page==6)||(page==7)||(page==5)){
              if((stepPressed)&&(stepPressedNum%16==4*col+row))
                stepPressed=false;
            }
            else if ((stepSequencer)&&((page==5)||(page==6)||((page==7)&&(STEPS==32)))) {
            }
            else
              midiNoteOnOff(false, (4*row+col)+(col*3-row*3)+page*16);
            //setBufferPixel(0,col,row);
          }
          buttons[row][col] = bState;
        }
      }
    }
  }
  interrupts();
}

// #####################################################################################################################
void readEditPots(){
  
  bool octaveChange=false;
  bool noteChange=false;
  bool velocityChange=false;
  bool probChange=false;
  
  int octaveValue=map(pot1->readValueAvr(octaveChange),0,127,0,9);
  int noteValue=map(pot2->readValueAvr(noteChange),0,127,0,msScaleLength[msCurrentScale[msSelectedChannel]]);  //-1
  noteValue=constrain(noteValue,0,msScaleLength[msCurrentScale[msSelectedChannel]]-1);
  int velocityValue=pot3->readValueAvr(velocityChange);
  int probValue=pot4->readValueAvr(probChange);
  
  if(octaveChange){ // pots readings changed
    if((stepSequencer)&&(stepEditMode)&&((page==6)||(page==7))){ // sequencing and edit mode enabled
    
      int currentOctave = msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep] / 12;
      int currentNote = msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep] % 12;
      
      if (currentOctave==octaveValue) { // give control to knob
        if((sequencerPaused)&&(!octaveKnobTakeover)){
          playPreviewNote(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], msStepVelocity[msSelectedChannel][msSelectedSequence/4][sellectedStep], msChannel[msSelectedChannel]);
        }
        octaveKnobTakeover=true;
      }
      
      if (octaveKnobTakeover) {
        if (currentOctave!=octaveValue) {
        // (Karg) MIDI.sendNoteOff(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], 0, msChannel[selectedChannel]);
        usbMIDI.sendNoteOff(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], 0, msChannel[selectedChannel]);
        msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep]=currentNote+octaveValue*12;
        lcdPrintInt(octaveValue);
        if(sequencerPaused){
          playPreviewNote(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], msStepVelocity[msSelectedChannel][msSelectedSequence/4][sellectedStep], msChannel[msSelectedChannel]);
        }
        }
      }
    }
  }
  
  if(noteChange){ // pots readings changed
    if((stepSequencer)&&(stepEditMode)&&((page==6)||(page==7))){ // sequencing and edit mode enabled
    
      int currentOctave = msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep] / 12;
      int currentNote = msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep] % 12;
      
      if (currentNote==scaleNotes[msCurrentScale[msSelectedChannel]][noteValue]) { // // give control to knob
        if((sequencerPaused)&&(!noteKnobTakeover)){
          playPreviewNote(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], msStepVelocity[msSelectedChannel][msSelectedSequence/4][sellectedStep], msChannel[msSelectedChannel]);
        }
        noteKnobTakeover=true;
      }
      
      if (noteKnobTakeover) {
        if (currentNote!=scaleNotes[msCurrentScale[msSelectedChannel]][noteValue]) {
        // (Karg) MIDI.sendNoteOff(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], 0, msChannel[selectedChannel]);
        usbMIDI.sendNoteOff(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], 0, msChannel[selectedChannel]);
        msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep]=scaleNotes[msCurrentScale[msSelectedChannel]][noteValue]+currentOctave*12;
        lcdPrintStr(noteStrings[scaleNotes[msCurrentScale[msSelectedChannel]][noteValue]]);
        if(sequencerPaused){
          playPreviewNote(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], msStepVelocity[msSelectedChannel][msSelectedSequence/4][sellectedStep], msChannel[msSelectedChannel]);
        }
       }
      }
    }
  }
  
  if(velocityChange){ // pots readings changed
    if((stepEditMode)&&((page==6)||(page==7))){ // sequencing and edit mode enabled
    
    int currentVel = msStepVelocity[msSelectedChannel][msSelectedSequence/4][sellectedStep];
    
    if(currentVel==velocityValue){
      vknob=true;
    }
    
    if(vknob){
      if(currentVel!=velocityValue){
        // (Karg) MIDI.sendNoteOff(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], 0, msChannel[selectedChannel]);
        usbMIDI.sendNoteOff(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], 0, msChannel[selectedChannel]);
        msStepVelocity[msSelectedChannel][msSelectedSequence/4][sellectedStep]=velocityValue;
        lcdPrintInt(velocityValue);
        if(sequencerPaused){
          playPreviewNote(msRootNote[msSelectedChannel]+msStepNote[msSelectedChannel][msSelectedSequence/4][sellectedStep], msStepVelocity[msSelectedChannel][msSelectedSequence/4][sellectedStep], msChannel[msSelectedChannel]);
        }
      }
    }
      
    }
  }
  
    if(probChange){ // pots readings changed
    if((stepEditMode)&&((page==6)||(page==7))){ // sequencing and edit mode enabled
    
    int currentProb = msStepChance[msSelectedChannel][msSelectedSequence/4][sellectedStep];
    
    if(currentProb==probValue){
      probknob=true;
    }
    
    if(probknob){
      if(currentProb!=probValue){
        msStepChance[msSelectedChannel][msSelectedSequence/4][sellectedStep]=probValue;
        lcdPrintInt(probValue);
      }
    }
    }
  }
}
// #####################################################################################################################
void initButtonpad() {
  for (int i=0; i<4; i++) {
    _buttonpad.pinMode(buttonRow[i], OUTPUT);
    _buttonpad.digitalWrite(buttonRow[i], HIGH);
    _buttonpad.pinMode(buttonCol[i], INPUT);
    _buttonpad.pullUp(buttonCol[i], HIGH);  // turn on a 100K pullup internally
  }
  for (int row=0; row<4; row++) {
    for (int col =0; col<4; col++) {
      buttons[row][col]=pow(2, col);
    }
  }
}

// #####################################################################################################################
// set display pins to initial state
void initDisplay() {
  for ( int i=0; i<4; i++) {
    _display.pinMode(dispGroundPins[i], OUTPUT);
    _display.pinMode(dispRedPins[i], OUTPUT);
    _display.pinMode(dispGreenPins[i], OUTPUT);
    _display.pinMode(dispBluePins[i], OUTPUT);

    _display.digitalWrite(dispGroundPins[i], HIGH);
    _display.digitalWrite(dispRedPins[i], LOW);
    _display.digitalWrite(dispGreenPins[i], LOW);
    _display.digitalWrite(dispBluePins[i], LOW);
  }
}

// #####################################################################################################################
// clear the pages array

void clearPages() {
  for (int p=0; p<8; p++) {
    for (int col=0; col <4; col++) {
      for (int row=0; row <4; row++) {
        displayPages[p][col][row]=0;
      }
    }
  }
}

// #####################################################################################################################
//set the specific pixel in the buffer to a specified color
void setPagePixel(int rgb, int p, int row, int col) {
  displayPages[p][col][row]=rgb%8;
}

void setPagePixel(int rgb, int p, int pixel) {
  int col = pixel % 4;
  int row = pixel / 4;
  displayPages[p][col][row]=rgb%8;
}

// #####################################################################################################################
//set the specific pixel in the buffer to a specified color
void setBufferPixel(int rgb, int row, int col) {
  displayBuffer[col][row]=rgb%8;
}

void setBufferPixel(int rgb, int pixel) {
  int col = pixel % 4;
  int row = pixel / 4;

  displayBuffer[col][row]=rgb%8;
}

// #####################################################################################################################
void setLedToA(int num) {
  num=map(num, 0, 127, 0, 16);
  constrain(num, 0, 15);
  clearBuffer();
  for (int i=0;i<num; i++) {
    setBufferPixel(BLUE, i);
  }
}

void setLedToB(int num) {
  num=map(num, 0, 127, 0, 16);
  constrain(num, 0, 15);
  clearBuffer();
  for (int i=0;i<num; i++) {
    setBufferPixel(GREEN, i);
  }
}

void setLedTo(int numa, int numb) {
  numa=map(numa, 0, 127, 0, 8);
  numb=map(numb, 0, 127, 0, 8);
  constrain(numa, 0, 7);
  constrain(numb, 0, 7);
  clearBuffer();
  for (int i=0; i<numa ; i++) {
    setBufferPixel(BLUE, i);
  }
  for (int i=0; i<numb ; i++) {
    setBufferPixel(GREEN, i+8);
  }
}

// #####################################################################################################################
void clearBuffer() {
  for (int i=0;i<4; i++)
    for (int j=0; j<4; j++)
      displayBuffer[i][j]=0;
}

// #####################################################################################################################
// function to print buffer on the display
void printBuffer(int pb[4][4]) {
  noInterrupts();
  int counter; 
  //_display.writeGPIOAB(61440);
  for (int col=0; col<4; col++) {
    counter = 61440 - (1<<dispGroundPins[col]);
    _display.writeGPIOAB(61440);

    for (int row=0; row<4; row++) {

      if ((pb[col][row]&(1<<0)) != 0) { // chech if theres red
        counter+=1<<dispRedPins[row];
      }
      if ((pb[col][row]&(1<<1)) != 0) { // chech if theres green
        counter+=1<<dispGreenPins[row];
      }
      if ((pb[col][row]&(1<<2)) != 0) { // chech if theres blue
        counter+=1<<dispBluePins[row];
      }
    }
    _display.writeGPIOAB(counter);
  }
  _display.writeGPIOAB(61440);
  interrupts();
}

// #####################################################################################################################
// function to print buffer on the display
void printBuffer() {
  noInterrupts();
  int counter; 
  //_display.writeGPIOAB(61440);
  for (int col=0; col<4; col++) {
    counter = 61440 - (1<<dispGroundPins[col]);
    _display.writeGPIOAB(61440);

    for (int row=0; row<4; row++) {

      if ((displayBuffer[col][row]&(1<<0)) != 0) { // chech if theres red
        counter+=1<<dispRedPins[row];
      }
      if ((displayBuffer[col][row]&(1<<1)) != 0) { // chech if theres green
        counter+=1<<dispGreenPins[row];
      }
      if ((displayBuffer[col][row]&(1<<2)) != 0) { // chech if theres blue
        counter+=1<<dispBluePins[row];
      }
    }
    _display.writeGPIOAB(counter);
  }
  _display.writeGPIOAB(61440);
  interrupts();
}

// #####################################################################################################################
void welcomeAnimation() {

  for ( int i=1; i<200; i++) {
    if (random(10)<5) {
      setBufferPixel(random(0, 7), random(0, 4), random(0, 4));
    }
    if (random(20)<2) {
      analogWrite(3, random(0, 64));
      analogWrite(4, random(0, 64));
      analogWrite(5, random(0, 64));
      analogWrite(6, random(0, 64));
    }
    if ( pageShift.update() ) {
    if ( pageShift.read() == LOW) {
      setupMode=true;
    }
    }
    delay(1);
    printBuffer();
  }
  clearBuffer();
  analogWrite(3, 0);
  analogWrite(4, 0);
  analogWrite(5, 0);
  analogWrite(6, 0);
  
  //if(setupMode)
  //  setupMenu();
}


//######################################################################################
void sevenSegUpdate(){
  
    for (int i = 0; i < 4; i++) {
    // take the latchPin low so 
    // the LEDs don't change while you're sending in bits:
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    //shiftOut(dataPin, clockPin, MSBFIRST, 0x0E<<1); // first
    //shiftOut(dataPin, clockPin, MSBFIRST, 0x0D<<1); // second
    //shiftOut(dataPin, clockPin, MSBFIRST, 0x0B<<1); // third
    shiftOut(dataPin, clockPin, MSBFIRST, ~(1<<(i%4))); // fourth
    //shiftOut(dataPin, clockPin, MSBFIRST, 0x0F<<1); // off
    if(showDigit[i]){
      if((i==2)&&(decimal))
        shiftOut(dataPin, clockPin, MSBFIRST, (digit[i])+0x80);  
      else
        shiftOut(dataPin, clockPin, MSBFIRST, (digit[i]));  
    }else{
      shiftOut(dataPin, clockPin, MSBFIRST, (0));
    }
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
    }
    
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, 0);
    shiftOut(dataPin, clockPin, MSBFIRST, 0);
    digitalWrite(latchPin, HIGH);
}

//######################################################################################
void lcdPrintInt(int num){
  if(num>9){
    if(num>99){
      if(num>999){
        showDigit[0]=true;
        digit[0]=sevencodes[num/1000];
        showDigit[1]=true;
        digit[1]=sevencodes[num/100%10];
        showDigit[2]=true;
        digit[2]=sevencodes[num/10%10];
        showDigit[3]=true;
        digit[3]=sevencodes[num%10];
      }
      else{
        showDigit[0]=false;
        showDigit[1]=true;
        digit[1]=sevencodes[num/100];
        showDigit[2]=true;
        digit[2]=sevencodes[num/10%10];
        showDigit[3]=true;
        digit[3]=sevencodes[num%10];
      }
    }else{
      showDigit[0]=false;
      showDigit[1]=false;
      showDigit[2]=true;
      digit[2]=sevencodes[num/10];
      showDigit[3]=true;
      digit[3]=sevencodes[num%10];
    }
  }
  else{
    showDigit[0]=false;
    showDigit[1]=false;
    showDigit[2]=false;
    showDigit[3]=true;
    digit[3]=sevencodes[num];
  }
  decimal=false;
}



//######################################################################################
void lcdPrintFloat(float num){
  if(num>0){
    if(num>9){
      if(num>99){
        showDigit[0]=true;
        digit[0]=sevencodes[(int)num/100%10];
        showDigit[1]=true;
        digit[1]=sevencodes[(int)num/10%10];
        showDigit[2]=true;
        digit[2]=sevencodes[(int)num%10];
        showDigit[3]=true;
        digit[3]=sevencodes[(int)(num*10)%10];
      }
      else{
        showDigit[0]=false;
        showDigit[1]=true;
        digit[1]=sevencodes[(int)num/10%10];
        showDigit[2]=true;
        digit[2]=sevencodes[(int)num%10];
        showDigit[3]=true;
        digit[3]=sevencodes[(int)(num*10)%10];
      }
    }else{
      showDigit[0]=false;
      showDigit[1]=false;
      showDigit[2]=true;
      digit[2]=sevencodes[(int)num%10];
      showDigit[3]=true;
      digit[3]=sevencodes[(int)(num*10)%10];
    }
  }
  else{
    showDigit[0]=false;
    showDigit[1]=false;
    showDigit[2]=true;
    digit[2]=sevencodes[0];
    showDigit[3]=true;
    digit[3]=sevencodes[(int)(num*10)%10];
  }
  decimal=true;
}




//######################################################################################
void lcdPrintStr(char const* str){
  
  showDigit[0]=false;
  showDigit[1]=false;
  showDigit[2]=false;
  showDigit[3]=false;
  
  for (int i = 0; i<4; i++){
    if(str[i]=='\0')
      break;
      
    if(str[i]==' ')
      digit[i]=sevencodes[10];
    else if(str[i]=='#')
      digit[i]=sevencodes[11];
    else
      digit[i]=sevencodes[(int)(str[i]-'a')+12];
    showDigit[i]=true;
  }
  decimal=false;
}
