// Functions:
// 
// void initLEDmatrix()                                - set display pins to initial state
// void initButtonpad()
// void readButtonpad()                                - the button matrix, shift, edit, track and seq are read in the main loop
// void readEditPots()
// void readEncoderA()
// void readEncoderB()
// void updateUI();
// void clearPages()                                   - clear the pages array
// void setPagePixel(int rgb, int p, int row, int col) - set the specific pixel in the buffer to a specified color
// void setBufferPixel(int rgb, int row, int col)      - set the specific pixel in the buffer to a specified color
// void setBufferPixel(int rgb, int pixel) 
// void clearBuffer() {
// void printBuffer(int pb[4][4])                      - function to print buffer on the display
// void printBuffer()                                  - function to print buffer on the display
// void welcomeAnimation() 
// void sevenSegUpdate()
// void lcdPrintInt(int num)
// void lcdPrintFloat(float num)
// void lcdPrintStr(char const* str)
// 
// #####################################################################################################################
// set LED matrix pins to initial state
void initLEDmatrix() {
  _ledmatrix.begin(LEDMATRIX_DEVICE_NUM);             // use default address 1
  for ( int i=0; i<4; i++) {
    _ledmatrix.pinMode(ledGroundPins[i], OUTPUT);
    _ledmatrix.pinMode(ledRedPins[i], OUTPUT);
    _ledmatrix.pinMode(ledGreenPins[i], OUTPUT);
    _ledmatrix.pinMode(ledBluePins[i], OUTPUT);

    _ledmatrix.digitalWrite(ledGroundPins[i], HIGH);
    _ledmatrix.digitalWrite(ledRedPins[i], LOW);
    _ledmatrix.digitalWrite(ledGreenPins[i], LOW);
    _ledmatrix.digitalWrite(ledBluePins[i], LOW);
  }
}

// #####################################################################################################################
void initButtonpad() {
  _buttonpad.begin(BUTTONPAD_DEVICE_NUM);             // use default address 0
  for (int i=0; i<4; i++) {
    _buttonpad.pinMode(buttonRow[i], OUTPUT);
    _buttonpad.digitalWrite(buttonRow[i], HIGH);
    _buttonpad.pinMode(buttonCol[i], INPUT);
    _buttonpad.pullUp(buttonCol[i], HIGH);            // turn on a 100K pullup internally
  }
  for (int row=0; row<4; row++) 
    for (int col =0; col<4; col++) {
      buttons[row][col]=true;
    }
}

void readButtonpad() {
  noInterrupts();
  int rowState;
  boolean buttonState;
  
  for (int row=0; row<4; row++) {                     // cycle through rows
    _buttonpad.writeGPIO(240-(1<<buttonRow[row]));
    rowState = _buttonpad.readGPIO();    
    for (int col=0; col<4; col++) {                   // ...and columns
      buttonState=rowState&(1<<col); 
      if((boolean) buttonState!=buttons[row][col])
        if ((millis() - buttonsBounce[row][col])>BOUNCE_DELAY) 
          if (!buttonState) buttonsBounce[row][col]=millis();  // button is pressed
          buttonsLast[row][col] = buttons[row][col];
          buttons[row][col] = buttonState;
    }
  }
  interrupts();
}

// #####################################################################################################################
void readEditPots(){
  octavePotValue=pot1->readValueAvr(octavePotChange);
  notePotValue=pot2->readValueAvr(notePotChange);
  velocityPotValue=pot3->readValueAvr(velocityPotChange);
  probPotValue=pot4->readValueAvr(probPotChange);
}

// #####################################################################################################################
void readEncoderA() {
  noInterrupts();
  encNewA[page*encodersBanked] = encA.read();
  
  if((stepEditMode)&&((page==SEQ_PAGE_1)||(page==SEQ_PAGE_2))){
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
      
//      encoderLedValueA[page*encodersBanked] = msStepChance[msSelectedVoice][msSelectedSequence/4][selectedStep];
//
//          // set the display encoder flag
//          if ((displayEnc==1)||(displayEnc==3)) // already displaying encoder
//            displayEnc=3; // we want to display both encoders
//          else
//            displayEnc=2; // just display one encoder
    }
  }

  if (encNewA[page*encodersBanked] != encOldA[page*encodersBanked]) {

    if (encNewA[page*encodersBanked]>encOldA[page*encodersBanked]) usbMIDI.sendControlChange(page*2*encodersBanked, 1, controlChannel);
    else if (encNewA[page*encodersBanked]<encOldA[page*encodersBanked]) usbMIDI.sendControlChange(page*2*encodersBanked, 127, controlChannel);
    
    encOldA[page*encodersBanked] = encNewA[page*encodersBanked];
    displayTimer=millis(); // set the time
  }
  interrupts();
}

// #####################################################################################################################
void readEncoderB() {
  noInterrupts();
  encNewB[page*encodersBanked] = encB.read();
  //encoderLedValueB[page] = constrain(newPositionB[page],0,127);
  
   if((stepEditMode)&&((page==SEQ_PAGE_1)||(page==SEQ_PAGE_2))){
    if (encNewB[page*encodersBanked]/4 != encOldB[page*encodersBanked]/4) {
      
      if (encNewB[page*encodersBanked]/4>encOldB[page*encodersBanked]/4) {
        msLength[msSelectedVoice]++;
      }else if (encNewB[page*encodersBanked]/4<encOldB[page*encodersBanked]/4){
        msLength[msSelectedVoice]--;
      }
      msLength[msSelectedVoice]=constrain(msLength[msSelectedVoice],2,steps);
      
      lcdPrintInt(msLength[msSelectedVoice]);
      
      // display step size
      if((msLength[msSelectedVoice]>16)&&(page==SEQ_PAGE_1))
       encoderLedValueB[page*encodersBanked] = 127;
      else if((msLength[msSelectedVoice]>16)&&(page==SEQ_PAGE_2))
        encoderLedValueB[page*encodersBanked] = (msLength[msSelectedVoice]-16)*8; //map(msLength[msSelectedVoice]-16, -1, 15,0,127);
      else if((msLength[msSelectedVoice]<17)&&(page==SEQ_PAGE_2))
        encoderLedValueB[page*encodersBanked] = 0;
      else if((msLength[msSelectedVoice]<17)&&(page==SEQ_PAGE_1))
        encoderLedValueB[page*encodersBanked] = msLength[msSelectedVoice]*8;

          // set the display encoder flag
          if ((displayEnc==2)||(displayEnc==3)) displayEnc=3; // already displaying encoder -> we want to display both encoders
          else displayEnc=1; // just display one encoder
    }
  }

  if (encNewB[page*encodersBanked] != encOldB[page*encodersBanked]) {
    if (encNewB[page*encodersBanked]>encOldB[page*encodersBanked]) usbMIDI.sendControlChange(1+page*2*encodersBanked, 1, controlChannel);
    else if (encNewB[page*encodersBanked]<encOldB[page*encodersBanked]) usbMIDI.sendControlChange(1+page*2*encodersBanked, 127, controlChannel);
    encOldB[page*encodersBanked] = encNewB[page*encodersBanked];
    displayTimer=millis(); // set the time
  }
  interrupts();
}

// #####################################################################################################################
void updateUI() {
        
  if (isShift) {                                                   // Shift button is pressed, handle shifts page
    for (int row=0; row<4; row++)
      for (int col=0; col<4; col++)
        if (buttons[row][col] != buttonsLast[row][col]){
          if (!buttons[row][col]){                                // ...a button was pressed
            int buttonNum=(4*row+col)+(col*3-row*3);      // 0: bottom-left, 3, bottom-right, 15: top-right
        
            if (buttonNum<8) {                            // change page (Duraion, Transition, Scale, MIDI Channel, Sequence 1-4, Songs)
              page = buttonNum;
              switch(buttonNum) { 
                case DURATION_PAGE:
                  lcdPrintStr("dur");
                  break;
                case TRANSP_PAGE:
                  lcdPrintStr("tran");
                  break;
                case SCALE_PAGE:
                  lcdPrintStr("scal");
                  break;
                case CHANNEL_PAGE:
                  lcdPrintStr("chan");
                  break;
                case SEQ_PAGE_1:
                case SEQ_PAGE_2:
                  lcdPrintStr("seqc");
                  break;
                case SONG_PAGE:
                  lcdPrintStr("song");
                  break;
              }
            }

           if(buttonNum==11) {                            // play direction
              //selectedPattern[selectedVoice]=p-8;
              msDirection[msSelectedVoice]=buttonNum-11;
            }

            if(buttonNum==12){                            // restart sequence
              if (!sequencerPaused){
                MIDI.sendSongPosition(0);
                MIDI.sendRealTime(Start);
              }else{
                MIDI.sendSongPosition(0);
                MIDI.sendRealTime(Stop);
              }
              //savePatterns(selectedVoice);
              clockCounter = 0;
              for(int i=0; i<4; i++){
                msCurrentStep[i]=0;
                checkStep(i);
              }
              //savePatternAnimation();
            }  

            if((buttonNum==13)&&(msHasPattern[msSelectedVoice][msSelectedSequence/4])){ // delete sequence
              //for(int j=0; j<4; j++)
                for(int i=0; i<32; i++){
                  msStepState[msSelectedVoice][msSelectedSequence/4][i]=false;
                  msStepNote[msSelectedVoice][msSelectedSequence/4][i]=48;
                  msStepVelocity[msSelectedVoice][msSelectedSequence/4][i]=0;
                    msStepChance[msSelectedVoice][msSelectedSequence/4][i]=127;
                  msStepLegato[msSelectedVoice][msSelectedSequence/4][i]=false;
                }
              //msLength[msSelectedVoice]=16;
              //msMuted[msSelectedVoice]=false;
            }
                  
            if((buttonNum==14)&&(msChannelHasPattern[msSelectedVoice])){                // mute channel
              msMuted[msSelectedVoice]=!msMuted[msSelectedVoice];
            }
        
            if (buttonNum==15){                           // start sequencer
              sequencerPaused=!sequencerPaused;
              if (!sequencerPaused) {
                MIDI.sendSongPosition(0);
                //usbMIDI.sendSongPosition(0);
                MIDI.sendRealTime(Start);
                //usbMIDI.sendRealTimeStart();
                clockCounter = 0; 
                setPagePixel(0, 6+currentStep/steps, currentStep%16);
                currentStep=0; 
                setPagePixel(BLUE, 6+(currentStep/steps), currentStep%16);
                checkStep(0);
                checkStep(1);
                checkStep(2);
                checkStep(3);
              }else{
                MIDI.sendSongPosition(0);
                MIDI.sendRealTime(Stop);
                //usbMIDI.sendSongPosition(0);
                //usbMIDI.sendRealTimeStop();
                //for(int i=0; i<4; i++){
                //  msCurrentStep[i]=0;
                //}
              }
            }

            //midiNoteOnOff(true,(4*row+col)+(col*3-row*3));
            //setBufferPixel(random(1,7),col,row);
          }
        }
    clearBuffer();
    
    for (int i=0; i<4; i++){
      msChannelHasPattern[i]=false;
       
      for(int k=0; k<4; k++){
        msHasPattern[i][k]=false; 
        for (int j=0; j<steps; j++){
          if (msStepState[i][k][j]){
            msHasPattern[i][k]=true;
            msChannelHasPattern[i]=true;
          }
        }
      }
  
      if(msHasPattern[msSelectedVoice][msSelectedSequence/4]) setBufferPixel(RED, 13);
      if(msChannelHasPattern[msSelectedVoice]){
        if(msMuted[msSelectedVoice]) setBufferPixel(YELLOW, 14);
        else setBufferPixel(GREEN, 14);
      }
      setBufferPixel(WHITE, 5);

      if (msCurrentStep[msSelectedVoice]<16) setBufferPixel(CYAN, 6);
      else setBufferPixel(BLUE, 6);

      if (steps==32) {
        if (msCurrentStep[msSelectedVoice]>15) setBufferPixel(CYAN, 7);
        else setBufferPixel(BLUE, 7);
      }
    
      if (!sequencerPaused) setBufferPixel(BLUE, 15);
      else setBufferPixel(YELLOW, 15);
    
      //show the selected pattern direction
      setBufferPixel(PINK, msDirection[msSelectedVoice]+8);
    }
    setBufferPixel(PINK, page);
    printBuffer(ledmatrixBuffer);
    displayEnc = 0; // stop displaying encoders
  
 
 
  } else if (displayEnc != 0) {                                    // normal sequencer mode???
    if (millis() - displayTimer > 200) displayEnc = 0; // display for a second
    else {
      switch (displayEnc) {
        case 3:  // show both
          setLedTo(encoderLedValueA[page * encodersBanked], encoderLedValueB[page * encodersBanked]);
          break;
        case 2:  // show B
          setLedToA(encoderLedValueA[page * encodersBanked]);
          break;
        case 1:  // show A
          setLedToB(encoderLedValueB[page * encodersBanked]);
          break;
      }
      printBuffer(ledmatrixBuffer);
    }


 
  } else {                                                          // shift is not pressed, go through all pages

    if (page==TRACKS_PAGE) {                                        // on tracks page...  
      for (int i = 0; i < VOICES; i++) {
        for (int j = 0; j < PATTERNS; j++) {
          if (msHasPattern[i][j]) setPagePixel(YELLOW, 5, j * 4 + i);
          else setPagePixel(0, 5, j * 4 + i);
        }

        if (msMuted[i]) setPagePixel(PINK, 5, msSelectedPattern[i] * 4 + i);
        else setPagePixel(GREEN, 5, msSelectedPattern[i] * 4 + i);

        if (stepEditMode) setPagePixel(RED, 5, msSelectedSequence);
        else setPagePixel(WHITE, 5, msSelectedSequence);
      }

      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]) {
            if (!buttons[row][col]){                                // ...a button was pressed
              setPagePixel(0, page, msSelectedSequence);
              setPagePixel(0, page, msSelectedPattern[row]*4+row);
              
              setPagePixel(0, 3, msChannel[msSelectedVoice]-1);
              setPagePixel(0, 2, msCurrentScale[msSelectedVoice]);
              setPagePixel(0, 1, msRootNote[msSelectedVoice]);
              
              if(msStepLength[msSelectedVoice]==1) setPagePixel(0, 0, 0);
              else if(msStepLength[msSelectedVoice]==2) setPagePixel(0, 0, 1);
              else if(msStepLength[msSelectedVoice]==4) setPagePixel(0, 0, 2);
              else if(msStepLength[msSelectedVoice]==8) setPagePixel(0, 0, 3);
              
              if(stepEditMode) {
                msSelectedSequence=4*col+row;
                msSelectedVoice=msSelectedSequence%4;
                
                if(!stepPressed) {
                  stepPressed=true;
                  stepPressedNum=msSelectedSequence;
                } else {
                  for(int i=0; i<STEPS; i++) {
                    msStepState[msSelectedSequence%4][msSelectedSequence/4][i]=msStepState[stepPressedNum%4][stepPressedNum/4][i];
                    msStepNote[msSelectedSequence%4][msSelectedSequence/4][i]=msStepNote[stepPressedNum%4][stepPressedNum/4][i];
                    msStepVelocity[msSelectedSequence%4][msSelectedSequence/4][i]=msStepVelocity[stepPressedNum%4][stepPressedNum/4][i];
                    msStepChance[msSelectedSequence%4][msSelectedSequence/4][i]=msStepChance[stepPressedNum%4][stepPressedNum/4][i];
                    msStepLegato[msSelectedSequence%4][msSelectedSequence/4][i]=msStepLegato[stepPressedNum%4][stepPressedNum/4][i];
                  }
                  msHasPattern[msSelectedSequence%4][msSelectedSequence/4]=msHasPattern[stepPressedNum%4][stepPressedNum/4];
                  msChannelHasPattern[msSelectedSequence%4]=msChannelHasPattern[stepPressedNum%4];
                }
              } else {
                msSelectedPattern[row]=col;
                msSelectedSequence=4*col+row;
                msSelectedVoice=msSelectedSequence%4;
              }
              //setPagePixel(WHITE, page, msSelectedPattern[row]*4+row);
            } else if((stepPressed)&&(stepPressedNum%16==4*col+row)) stepPressed=false; // ...a button was released
          }
    } 
  
    else if (page==CHANNEL_PAGE) {                    // on channel page...
      setPagePixel(PINK, page, msChannel[msSelectedVoice] - 1);
      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]) 
            if (!buttons[row][col]){                  // ...a button was pressed, so select a channel
              setPagePixel(0, page, msChannel[msSelectedVoice]-1);
              msChannel[msSelectedVoice]=4*col+row+1;
              setPagePixel(PINK, page, msChannel[msSelectedVoice]-1);
              lcdPrintInt(msChannel[msSelectedVoice]);
            }
    }
            
     else if(page==SONG_PAGE) {                        // on song page
      /* KARG: 
        load/save song, does not need its own page
        the encoder change the song number (which is displayed on the lcd
        one encoder (red illuminated) saves
        the other (green illuminated) loads the song at the given number
      */              
      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]) 
            if (!buttons[row][col]){                   // ...a button was pressed
              int p=4*col+row;	
              setPagePixel(YELLOW, page, song);        // new song was selected, turn old button to yellow
              //turn off settings leds
              //setPagePixel(0, 3, msChannel[msSelectedVoice]-1);
              //setPagePixel(0, 2, msCurrentScale[msSelectedVoice]);
              //setPagePixel(0, 1, msRootNote[msSelectedVoice]);
	  			
              //if(msStepLength[msSelectedVoice]==1) setPagePixel(0, 0, 0);
              //else if(msStepLength[msSelectedVoice]==2) setPagePixel(0, 0, 1);
              //else if(msStepLength[msSelectedVoice]==4) setPagePixel(0, 0, 2);
              //else if(msStepLength[msSelectedVoice]==8) setPagePixel(0, 0, 3);
              switch(p) {
                case 0:
                case 1:
                case 2:                                //load song;
                case 3:
                  song=p;
                  if (sdPresent) loadPatternsSd(song);
                  else loadPatternsEe(song);
                  break;
                case 12:                               // save
                  if (sdPresent) savePatternsSd(song);
                  else savePatternsEe(song);
                  break;
              }
              setPagePixel(GREEN, page, song);
            }
    }
    
    else if (page==SCALE_PAGE){                         // on scale page...
      setPagePixel(CYAN, page, msCurrentScale[msSelectedVoice]);
      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]){
            if (!buttons[row][col]){                   // ...a button was pressed, select sequence scale

              setPagePixel(0, page, msCurrentScale[msSelectedVoice]);
              if(stepEditMode) msCurrentScale[msSelectedVoice]=constrain(4*col+row,0,11);
              else for(int i=0;i<4;i++) msCurrentScale[i]=constrain(4*col+row,0,11);
              setPagePixel(CYAN, page, msCurrentScale[msSelectedVoice]);
              if(sequencerPaused){
                for(int i=0; i<msScaleLength[msCurrentScale[msSelectedVoice]]-1; i++){
                  playPreviewNote(msRootNote[msSelectedVoice]+48+scaleNotes[msCurrentScale[msSelectedVoice]][i], 127, msChannel[msSelectedVoice]);
                  delay(110);
                }  
                for(int i=msScaleLength[msCurrentScale[msSelectedVoice]]-1; i>=0; i--){
                  playPreviewNote(msRootNote[msSelectedVoice]+48+scaleNotes[msCurrentScale[msSelectedVoice]][i], 127, msChannel[msSelectedVoice]);
                  delay(110);
                }
              }
              // fix scale notes;
              if(stepEditMode)                         // fix only for this voice{
                for(int p=0;p<PATTERNS;p++)
                  for(int s=0;s<STEPS;s++)
                    while(!noteInScale(msStepNote[msSelectedVoice][p][s],msCurrentScale[msSelectedVoice])) msStepNote[msSelectedVoice][p][s]++;     
              else                                     // fix for all voices
                for(int v=0;v<VOICES;v++)
                  for(int p=0;p<PATTERNS;p++)
                    for(int s=0;s<STEPS;s++)
                      while(!noteInScale(msStepNote[v][p][s],msCurrentScale[v])) msStepNote[v][p][s]++;            
              lcdPrintStr(scalesStrings[msCurrentScale[msSelectedVoice]]);
            }
          }
  
    } else if (page==TRANSP_PAGE){                       // on scale page...
     setPagePixel(YELLOW, page, msRootNote[msSelectedVoice]);
       for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]) 
            if (!buttons[row][col]){                   // ...a button was pressed, select sequence root note
              if(col==3){                              // change octave
                if(row==0)//down
                  for(int p=0;p<4;p++)
                    for(int s=0;s<32;s++)
                      if((msStepNote[msSelectedVoice][p][s]/12)>0) msStepNote[msSelectedVoice][p][s]-=12;
                if(row==1)//up
                  for(int p=0;p<4;p++)
                    for(int s=0;s<32;s++)
                      if((msStepNote[msSelectedVoice][p][s]/12)<9) msStepNote[msSelectedVoice][p][s]+=12;			
              } else {
                setPagePixel(0, page, msRootNote[msSelectedVoice]);
                if(stepEditMode) msRootNote[msSelectedVoice]=constrain(4*col+row,0,11);
                else for(int i=0; i<4; i++) msRootNote[i]=constrain(4*col+row,0,11);
                setPagePixel(YELLOW, page, msRootNote[msSelectedVoice]);
                if(sequencerPaused) playPreviewNote(msRootNote[msSelectedVoice]+48, 127, msChannel[msSelectedVoice]);   
                lcdPrintStr(noteStrings[msRootNote[msSelectedVoice]]);
              }
	    }		  
    } 
  
    else if (page==DURATION_PAGE) {                    // on duration page...
      if (msStepLength[msSelectedVoice] == 1) {
        setPagePixel(GREEN, page, 0);
      } else if (msStepLength[msSelectedVoice] == 2) {
        setPagePixel(GREEN, page, 1);
      } else if (msStepLength[msSelectedVoice] == 4) {
        setPagePixel(GREEN, page, 2);
      } else if (msStepLength[msSelectedVoice] == 8) {
        setPagePixel(GREEN, page, 3);
      }

      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]) 
            if (!buttons[row][col]){                   // ...a button was pressed, select sequence length
 
              if(msStepLength[msSelectedVoice]==1) setPagePixel(0, page, 0);
              else if(msStepLength[msSelectedVoice]==2) setPagePixel(0, page, 1);
              else if(msStepLength[msSelectedVoice]==4) setPagePixel(0, page, 2);
              else if(msStepLength[msSelectedVoice]==8) setPagePixel(0, page, 3);
            
              if(4*col+row==0)  msStepLength[msSelectedVoice]=1;
              else if(4*col+row==1) msStepLength[msSelectedVoice]=2;
              else if(4*col+row==2) msStepLength[msSelectedVoice]=4;
              else if(4*col+row==3) msStepLength[msSelectedVoice]=8;
             
              if(msStepLength[msSelectedVoice]==1){
                setPagePixel(GREEN, page, 0);
                lcdPrintInt(16);
              }else if(msStepLength[msSelectedVoice]==2){
                setPagePixel(GREEN, page, 1);
                lcdPrintInt(8);
              }else if(msStepLength[msSelectedVoice]==4){
                setPagePixel(GREEN, page, 2);
                lcdPrintInt(4);
              }else if(msStepLength[msSelectedVoice]==8){
                setPagePixel(GREEN, page, 3);
                lcdPrintInt(2);
              }
            }
    }
  
    else if (page==TRACKS_PAGE){                       // on tracks (=voice) page...
      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]) 
            if (!buttons[row][col]){                   // ...a button was pressed, select a voice
              setPagePixel(0, page, selectedVoice);
              selectedVoice=4*col+row;
              setPagePixel(WHITE, page, selectedVoice);
            }
    }

    else if ((page==SEQ_PAGE_1)||(page==SEQ_PAGE_2)){                        // on sequence pages...
      if(stepEditMode){                                                      // ...with edit mode enabled...
        if(octavePotChange){                                                 // ...octave pot readings changed  
          int octaveValue=map(octavePotValue,0,127,0,9);      
          int currentOctave = msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep] / 12;
          int currentNote = msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep] % 12;
          if (currentOctave==octaveValue) {                                  // give control to knob?
            if((sequencerPaused)&&(!octavePotTakeover)) 
              playPreviewNote(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], msStepVelocity[msSelectedVoice][msSelectedSequence/4][selectedStep], msChannel[msSelectedVoice]);
            octavePotTakeover=true;
          }
          if (octavePotTakeover) {
            if (currentOctave!=octaveValue) {
              MIDI.sendNoteOff(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], 0, msChannel[selectedVoice]);
              usbMIDI.sendNoteOff(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], 0, msChannel[selectedVoice]);
              msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep]=currentNote+octaveValue*12;
              lcdPrintInt(octaveValue);
              if(sequencerPaused)
                playPreviewNote(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], msStepVelocity[msSelectedVoice][msSelectedSequence/4][selectedStep], msChannel[msSelectedVoice]);    
            }
          } else lcdPrintInt(currentOctave);
        }
      
        if(notePotChange){                                                   // ... note pot readings changed
          int noteValue=map(notePotValue,0,127,0,msScaleLength[msCurrentScale[msSelectedVoice]]);  //-1
          noteValue=constrain(noteValue,0,msScaleLength[msCurrentScale[msSelectedVoice]]-1);
          int currentOctave = msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep] / 12;
          int currentNote = msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep] % 12;
          if (currentNote==scaleNotes[msCurrentScale[msSelectedVoice]][noteValue]) { // give control to knob
            if((sequencerPaused)&&(!notePotTakeover))
              playPreviewNote(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], msStepVelocity[msSelectedVoice][msSelectedSequence/4][selectedStep], msChannel[msSelectedVoice]);
            notePotTakeover=true;
          }
          if (notePotTakeover) {
            if (currentNote!=scaleNotes[msCurrentScale[msSelectedVoice]][noteValue]) {
              MIDI.sendNoteOff(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], 0, msChannel[selectedVoice]);
              usbMIDI.sendNoteOff(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], 0, msChannel[selectedVoice]);
              msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep]=scaleNotes[msCurrentScale[msSelectedVoice]][noteValue]+currentOctave*12;
              lcdPrintStr(noteStrings[scaleNotes[msCurrentScale[msSelectedVoice]][noteValue]]);
              if(sequencerPaused)
                playPreviewNote(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], msStepVelocity[msSelectedVoice][msSelectedSequence/4][selectedStep], msChannel[msSelectedVoice]);
            }
          }
        }
    
        if(velocityPotChange){                                               // ... velocity pot readings changed
          int currentVel = msStepVelocity[msSelectedVoice][msSelectedSequence/4][selectedStep];
          if(currentVel==velocityPotValue) velocityPotTakeover=true;
            if(velocityPotTakeover){
              if(currentVel!=velocityPotValue){
                MIDI.sendNoteOff(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], 0, msChannel[selectedVoice]);
                usbMIDI.sendNoteOff(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], 0, msChannel[selectedVoice]);
                msStepVelocity[msSelectedVoice][msSelectedSequence/4][selectedStep]=velocityPotValue;
                lcdPrintInt(velocityPotValue);
                if(sequencerPaused) playPreviewNote(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep], msStepVelocity[msSelectedVoice][msSelectedSequence/4][selectedStep], msChannel[msSelectedVoice]);  
              }
            }
          }
  
          if(probPotChange){                                                 // ... probalbility pot readings changed
            int currentProb = msStepChance[msSelectedVoice][msSelectedSequence/4][selectedStep];
            if(currentProb==probPotValue) probPotTakeover=true;
            if(probPotTakeover){
              if(currentProb!=probPotValue){
                msStepChance[msSelectedVoice][msSelectedSequence/4][selectedStep]=probPotValue;
                lcdPrintInt(map(probPotValue,0,127,0,100));
              }
            }
          }
      }

      byte offset = 0;
      if (page==SEQ_PAGE_1){                        // on sequence page 1...
        for (int i = 0; i < steps; i++) {
          if ((stepEditMode) && (selectedStep == i) && (i < 16)) {
            setPagePixel(RED, 6, i % 16);
          } else if ((msStepState[msSelectedVoice][msSelectedSequence / 4][i]) && (i < 16)) {
            setPagePixel(GREEN, 6, i % 16);
          } else if ((msStepLegato[msSelectedVoice][msSelectedSequence / 4][i]) && (i < 16)) {
            setPagePixel(YELLOW, 6, i % 16);
          } else if (i < 16) {
            setPagePixel(0, 6, i % 16);
          }
        }
        if (msCurrentStep[msSelectedVoice] < 16) setPagePixel(BLUE, 6, msCurrentStep[msSelectedVoice] % msLength[msSelectedVoice] % 16); //currentStep%midiSeqOutLength[selectedVoice]%16
      }  

      if ((page==SEQ_PAGE_2)&&(steps==32)) { // on sequence page 2...        
        for (int i = 0; i < steps; i++) {
          if ((stepEditMode) && (selectedStep == i) && (i > 15)) setPagePixel(RED, 7, i % 16);
          else if ((msStepState[msSelectedVoice][msSelectedSequence / 4][i]) && (i > 15)) {
            setPagePixel(GREEN, 7, i % 16);
          } else if ((msStepLegato[msSelectedVoice][msSelectedSequence / 4][i]) && (i > 15)) {
            setPagePixel(YELLOW, 7, i % 16);
          } else if (i > 15) {
            setPagePixel(0, 7, i % 16);
          }
        }
        if (msCurrentStep[msSelectedVoice] > 15) setPagePixel(BLUE, 7, msCurrentStep[msSelectedVoice] % 16); //currentStep%midiSeqOutLength[selectedVoice]%16
        offset = 16;
      }
        
        
        
        
        for (int row=0; row<4; row++)
          for (int col=0; col<4; col++)
            if (buttons[row][col] != buttonsLast[row][col]){ 
              if (!buttons[row][col]){                   // ...a button was pressed, enter pattern
                if(!stepEditMode){
                  //stepState[selectedPattern[selectedVoice]][selectedVoice][4*col+row+offset]=!stepState[selectedPattern[selectedVoice]][selectedVoice][4*col+row+offset];
                  msStepState[msSelectedVoice][msSelectedSequence/4][4*col+row+offset]=!msStepState[msSelectedVoice][msSelectedSequence/4][4*col+row+offset];
                  if(msStepState[msSelectedVoice][msSelectedSequence/4][4*col+row+offset]){
                    bool velocityChange=false;
                    msStepVelocity[msSelectedVoice][msSelectedSequence/4][4*col+row+offset]=pot3->readValueAvr(velocityChange);
                  }else msStepVelocity[msSelectedVoice][msSelectedSequence/4][4*col+row+offset]=0;
                }else{
                  if(!stepPressed){
                    stepPressed=true;
                    stepPressedNum=4*col+row+offset;
                  }else{
                    if(4*col+row+offset-stepPressedNum>0)
                      for(int i=stepPressedNum+1; i<=4*col+row+offset; i++) msStepLegato[msSelectedVoice][msSelectedSequence/4][i]=!msStepLegato[msSelectedVoice][msSelectedSequence/4][i];
                    else
                    for(int i=4*col+row+1; i<=stepPressedNum; i++) msStepLegato[msSelectedVoice][msSelectedSequence/4][i]=!msStepLegato[msSelectedVoice][msSelectedSequence/4][i];
                  }
                }
                selectedStep=4*col+row+offset;
                octavePotTakeover=false;
                notePotTakeover=false;
                velocityPotTakeover=false;
                probPotTakeover=false;
                lcdPrintStr(noteStrings[(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedSequence/4][selectedStep]) % 12]);
              } else if((stepPressed)&&(stepPressedNum%16==4*col+row)) stepPressed=false;   // ...button was released  
            }
     
            
    }
    
    else{                                              // on none of these pages...
      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]){ 
            if (!buttons[row][col]){                   // ...a button was pressed, send note on MIDI command
              midiNoteOnOff(true, (4*row+col)+(col*3-row*3)+page*16);  
              //setBufferPixel(random(1,7),col,row);
            } else {                                   // a button was released, sned note off MIDI command
              midiNoteOnOff(false, (4*row+col)+(col*3-row*3)+page*16);
              //setBufferPixel(0,col,row);
            }
          }
    }
    printBuffer(displayPages[page]);
  }
}

// #####################################################################################################################
// clear the pages array
void clearPages() {
  for (int p=0; p<8; p++) 
    for (int col=0; col <4; col++) 
      for (int row=0; row <4; row++) 
        displayPages[p][col][row]=0;
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
  ledmatrixBuffer[col][row]=rgb%8;
}

void setBufferPixel(int rgb, int pixel) {
  int col = pixel % 4;
  int row = pixel / 4;
  ledmatrixBuffer[col][row]=rgb%8;
}

// #####################################################################################################################
void setLedToA(int num) {
  num=map(num, 0, 127, 0, 16);
  constrain(num, 0, 15);
  clearBuffer();
  for (int i=0;i<num; i++) setBufferPixel(BLUE, i);
}

void setLedToB(int num) {
  num=map(num, 0, 127, 0, 16);
  constrain(num, 0, 15);
  clearBuffer();
  for (int i=0;i<num; i++) setBufferPixel(GREEN, i);
}

void setLedTo(int numa, int numb) {
  numa=map(numa, 0, 127, 0, 8);
  numb=map(numb, 0, 127, 0, 8);
  constrain(numa, 0, 7);
  constrain(numb, 0, 7);
  clearBuffer();
  for (int i=0; i<numa ; i++) setBufferPixel(BLUE, i);
  for (int i=0; i<numb ; i++) setBufferPixel(GREEN, i+8);
}

// #####################################################################################################################
void clearBuffer() {
  for (int i=0;i<4; i++)
    for (int j=0; j<4; j++)
      ledmatrixBuffer[i][j]=0;
}

// #####################################################################################################################
// function to print buffer on the display
void printBuffer(int pb[4][4]) {
  noInterrupts();
  int counter; 
  //_ledmatrix.writeGPIOAB(61440);
  for (int col=0; col<4; col++) {
    counter = 61440 - (1<<ledGroundPins[col]);
    _ledmatrix.writeGPIOAB(61440);

    for (int row=0; row<4; row++) {
      if ((pb[col][row]&(1<<0)) != 0) counter+=1<<ledRedPins[row]; // check if theres red
      if ((pb[col][row]&(1<<1)) != 0) counter+=1<<ledGreenPins[row]; // check if theres green
      if ((pb[col][row]&(1<<2)) != 0) counter+=1<<ledBluePins[row]; // check if theres blue
    }
    _ledmatrix.writeGPIOAB(counter);
  }
  _ledmatrix.writeGPIOAB(61440);
  interrupts();
}

// #####################################################################################################################
// function to print buffer on the display
void printBuffer() {
  noInterrupts();
  int counter; 
  //_ledmatrix.writeGPIOAB(61440);
  for (int col=0; col<4; col++) {
    counter = 61440 - (1<<ledGroundPins[col]);
    _ledmatrix.writeGPIOAB(61440);
    for (int row=0; row<4; row++) {
      if ((ledmatrixBuffer[col][row]&(1<<0)) != 0) counter+=1<<ledRedPins[row]; // check if theres red
      if ((ledmatrixBuffer[col][row]&(1<<1)) != 0) counter+=1<<ledGreenPins[row]; // check if theres green
      if ((ledmatrixBuffer[col][row]&(1<<2)) != 0) counter+=1<<ledBluePins[row]; // check if theres blue
    }
    _ledmatrix.writeGPIOAB(counter);
  }
  _ledmatrix.writeGPIOAB(61440);
  interrupts();
}

// #####################################################################################################################
void welcomeAnimation() {

  for ( int i=1; i<200; i++) {
    if (random(10)<5) setBufferPixel(random(0, 7), random(0, 4), random(0, 4));
    if (random(20)<2) {
      analogWrite(3, random(0, 64));
      analogWrite(4, random(0, 64));
      analogWrite(5, random(0, 64));
      analogWrite(6, random(0, 64));
    }
    if ( shiftButton.update() ) 
      if ( shiftButton.read() == LOW) setupMode=true;
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
    // take the latch pin low so 
    // the LEDs don't change while you're sending in bits:
    digitalWrite(LCD_LATCH_PIN, LOW);
    // shift out the bits:
    //shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, 0x0E<<1); // first
    //shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, 0x0D<<1); // second
    //shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, 0x0B<<1); // third
    shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, ~(1<<(i%4))); // fourth
    //shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, 0x0F<<1); // off
    if(showDigit[i]){
      if((i==2)&&(decimal)) shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, (digit[i])+0x80);  
      else shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, (digit[i]));  
    }else shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, (0));
    //take the latch pin high so the LEDs will light up:
    digitalWrite(LCD_LATCH_PIN, HIGH);
  }
  digitalWrite(LCD_LATCH_PIN, LOW);
  shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, 0);
  shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, 0);
  digitalWrite(LCD_LATCH_PIN, HIGH);
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
      }else{
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
  }else{
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
      }else{
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
  }else{
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
    if(str[i]=='\0') break;
    if(str[i]==' ') digit[i]=sevencodes[10];
    else if(str[i]=='#') digit[i]=sevencodes[11];
    else if(str[i]>47 && str[i]<58) digit[i]=sevencodes[str[i]-'0'];
    else digit[i]=sevencodes[(int)(str[i]-'a')+12];
    showDigit[i]=true;
  }
  decimal=false;
}
