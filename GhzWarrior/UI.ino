// Functions:
// 
// void initLEDmatrix()                                         - set display pins to initial state
// void initButtonpad()
// void readButtonpad()                                         - the button matrix, shift, edit, track and seq are read in the main loop
// void readEditPots()
// void readEncoderA()
// void readEncoderB()
// void updateUI();
// void clearPages()                                            - clear the pages array
// void setPagePixel(int rgb, int p, int row, int col)          - set the specific pixel in the buffer to a specified color
// void setBufferPixel(int rgb, int row, int col)               - set the specific pixel in the buffer to a specified color
// void setBufferPixel(int rgb, int pixel) 
// void clearBuffer() {
// void printBuffer(int pb[4][4])                               - function to print buffer on the display
// void printBuffer()                                           - function to print buffer on the display
// void welcomeAnimation() 
// void sevenSegUpdate()
// void lcdPrintInt(int num)                                    - shows an int on the LCD (and turns the unused space black)
// void lcdPrintDigit(byte num, byte digit)                     - shows a single digit without affecting the rest
// void lcdPrintTwo(byte num1, byte num2)                       - shows two values from 0-99 (and turns unused space to black)
// void lcdPrintFloat(float num)                                - displays a float on the lcd
// void lcdPrintStr(char const* str, boolean important = false) - displays a string (turns the unused space to black)
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
  
  //setPagePixel(WHITE, TRANSP_PAGE, 12);      // KARG_ WHY??
  //setPagePixel(WHITE, TRANSP_PAGE, 13);      // KARG_ WHY??
  setPagePixel(GREEN, SONG_PAGE, 0);
  setPagePixel(YELLOW, SONG_PAGE, 1);
  setPagePixel(YELLOW, SONG_PAGE, 2);
  setPagePixel(RED, SONG_PAGE, 12);
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
      buttonsLast[row][col] = buttons[row][col];
      if((boolean) buttonState!=buttons[row][col]) {
        if ((millis() - buttonsBounce[row][col])>BOUNCE_DELAY) {
          buttonsBounce[row][col]=millis();
          buttons[row][col] = buttonState;
        }
      }
    }
  }
  interrupts();
}

// #####################################################################################################################
void readEditPots(){
  notePotValue=pot1->readValueAvr(notePotChange);
  octavePotValue=pot2->readValueAvr(octavePotChange);
  velocityPotValue=pot3->readValueAvr(velocityPotChange);
  probPotValue=pot4->readValueAvr(probPotChange);
  if ((octavePotChange) || (notePotChange) || (velocityPotChange) || (probPotChange)) lcdLastTime = 0;  // reset the display timer to show new pot values
}

// #####################################################################################################################
void readEncoderA() {
/*  noInterrupts();
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
      
//      encoderLedValueA[page*encodersBanked] = msStepChance[msSelectedVoice][msSelectedPattern][selectedStep];
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
  interrupts();*/
}

// #####################################################################################################################
void readEncoderB() {
/*  noInterrupts();
  encNewB[page*encodersBanked] = encB.read();
  //encoderLedValueB[page] = constrain(newPositionB[page],0,127);
  
   if((stepEditMode)&&((page==SEQ_PAGE_1)||(page==SEQ_PAGE_2))){
    if (encNewB[page*encodersBanked]/4 != encOldB[page*encodersBanked]/4) {
      
      if (encNewB[page*encodersBanked]/4>encOldB[page*encodersBanked]/4) {
        msLength[msSelectedVoice]++;
      }else if (encNewB[page*encodersBanked]/4<encOldB[page*encodersBanked]/4){
        msLength[msSelectedVoice]--;
      }
      msLength[msSelectedVoice]=constrain(msLength[msSelectedVoice],2,STEPS);
      
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
  interrupts();*/
}

// #####################################################################################################################
void updateUI() {
        
  if (isShift) {                                                   // Shift button is pressed, handle shifts page    
    lcdPrintStr("vo");                                             // display the current voice (since most actions will be for that)
    lcdPrintDigit(selectedVoice+1,3);
    for (int row=0; row<4; row++)
      for (int col=0; col<4; col++)
        if (buttons[row][col] != buttonsLast[row][col]){
          int buttonNum=(4*row+col)+(col*3-row*3);                 // 0: bottom-left, 3, bottom-right, 15: top-right   
          
          if (!buttons[row][col]){                                 // ...a button was pressed     
            if (buttonNum<=TRACKS_PAGE) {                          // change page (Duraion, Transition, Scale, MIDI Channel, Sequence 1-4, Songs)
              page = buttonNum;
              switch(buttonNum) { 
                case DURATION_PAGE:
                  lcdPrintStr("dur", true);
                  break;
                case TRANSP_PAGE:
                  lcdPrintStr("tran", true);
                  break;
                case SCALE_PAGE:
                  lcdPrintStr("scal", true);
                  break;
                case CHANNEL_PAGE:
                  lcdPrintStr("chan", true);
                  break;                
                case SEQ_PAGE_1:
                  lcdPrintStr("sen1", true);
                  break; 
                case SEQ_PAGE_2:
                  lcdPrintStr("sen2", true);
                  break; 
                case SEQ_PAGE_3:
                  lcdPrintStr("sen3", true);
                  break; 
                case SEQ_PAGE_4:
                  lcdPrintStr("sen4", true);
                  break; 
                case SONG_PAGE:               
                  lcdPrintStr("song", true);
                  break;                
                case TRACKS_PAGE:               
                  lcdPrintStr("trck", true);
                  break;                
              }
            }
                        
            if (buttonNum == DIRECTION_BUTTON) {                  // cycle through ply directions
              msDirection[msSelectedVoice]++;
              if (msDirection[msSelectedVoice]>3) msDirection[msSelectedVoice] = 0;
              switch(msDirection[msSelectedVoice]) { 
                case FORWARD:
                  lcdPrintStr("for");
                  break;
                case BACKWARD:
                  lcdPrintStr("bac");
                  break;
                case PINGPONG:
                  lcdPrintStr("bnc");
                  break;
                case RANDOM:
                  lcdPrintStr("ran");
                  break;
              }
            }

            if(buttonNum==RESTART_BUTTON){                            // restart sequence
              if (!sequencerPaused){
                MIDI.sendSongPosition(0);
                MIDI.sendRealTime(Start);
                usbMIDI.sendRealTime(Start);
                // KARG???
                lcdPrintStr("play");
              }else{
                MIDI.sendSongPosition(0);
                MIDI.sendRealTime(Stop);
                usbMIDI.sendRealTime(Stop);
                lcdPrintStr("Stop");
                // karg: ???
              }
              //savePatterns(selectedVoice);
              clockCounter = 0;
              for(int v=0; v<VOICES; v++){
                msCurrentStep[v]=0;
                checkStep(v);
              }
              //savePatternAnimation();
            }  

            if((buttonNum==CLEAR_BUTTON)&&(msHasPattern[msSelectedVoice][msSelectedPattern])){ // delete sequence
              //for(int p=0; p<PATTERNS; p++)
                for(int s=0; s<STEPS; s++){
                  msStepNote[msSelectedVoice][msSelectedPattern][s]=48;
                  msStepVelocity[msSelectedVoice][msSelectedPattern][s]=0;
                  msStepChance[msSelectedVoice][msSelectedPattern][s]=127;
                }
              //msLength[msSelectedVoice]=16;
              //msMuted[msSelectedVoice]=false;
            }
                  
            if((buttonNum==MUTE_BUTTON)) msMuted[msSelectedVoice]=!msMuted[msSelectedVoice]; // mute voice
              
            if (buttonNum==PLAY_BUTTON){                           // start sequencer
              sequencerPaused=!sequencerPaused;
              if (!sequencerPaused) {
                MIDI.sendSongPosition(0);
                //usbMIDI.sendSongPosition(0);
                MIDI.sendRealTime(Start);
                usbMIDI.sendRealTime(Start);
                mtcTimer.begin(midiSendMTC, mtcInterval);
                clockCounter = 0; 
                setPagePixel(OFF, SEQ_PAGE_1+currentStep/STEPS, currentStep%16);
                currentStep=0; 
                setPagePixel(BLUE, SEQ_PAGE_1+(currentStep/STEPS), currentStep%16);
                for (int v=0;v<VOICES;v++) checkStep(v);
                lcdPrintStr("play");
              }else{
                MIDI.sendSongPosition(0);
                MIDI.sendRealTime(Stop);
                usbMIDI.sendRealTime(Start);                
                mtcTimer.end();
                lcdPrintStr("stop");
                //usbMIDI.sendSongPosition(0);
                //usbMIDI.sendRealTimeStop();
                //for(int i=0; i<4; i++){
                //  msCurrentStep[i]=0;
                //}
              }
            }

            if ((buttonNum==SEQ_PAGE_1) || (buttonNum==SEQ_PAGE_2) || (buttonNum==SEQ_PAGE_3) || (buttonNum==SEQ_PAGE_4)){                           // copy/paste sequencer page 

              if (!buttonPressed){
                buttonPressed = true;
                buttonPressedNum = buttonNum;
              }else{
                lcdPrintStr("copy", true);
                for(int s=0; s<16; s++) {
                  msStepNote[msSelectedVoice][msSelectedPattern][s+(buttonNum%4)*16]=msStepNote[msSelectedVoice][msSelectedPattern][s+(buttonPressedNum%4)*16];
                  msStepVelocity[msSelectedVoice][msSelectedPattern][s+(buttonNum%4)*16]=msStepVelocity[msSelectedVoice][msSelectedPattern][s+(buttonPressedNum%4)*16];
                  msStepChance[msSelectedVoice][msSelectedPattern][s+(buttonNum%4)*16]=msStepChance[msSelectedVoice][msSelectedPattern][s+(buttonPressedNum%4)*16];
                  msStepCC[msSelectedVoice][msSelectedPattern][s+(buttonNum%4)*16]=msStepCC[msSelectedVoice][msSelectedPattern][s+(buttonPressedNum%4)*16];
                  msStepCCValue[msSelectedVoice][msSelectedPattern][s+(buttonNum%4)*16]=msStepCCValue[msSelectedVoice][msSelectedPattern][s+(buttonPressedNum%4)*16];
                }
              }
            } 

            //midiNoteOnOff(true,(4*row+col)+(col*3-row*3));
            //setBufferPixel(random(1,7),col,row);
          } else if((buttonPressed)&&(buttonPressedNum==buttonNum)) buttonPressed=false; // ...a button was released
        }
    clearBuffer();
    
    for (int v=0; v<VOICES; v++){                                  // to display correct color information, determine...
      msVoiceHasPattern[v]=false;
      for(int p=0; p<PATTERNS; p++){
        msHasPattern[v][p]=false; 
        for (int s=0; s<STEPS; s++){
          if (msStepVelocity[v][p][s]&0x80){
            msHasPattern[v][p]=true;                               // ...which pattern has active steps
            msVoiceHasPattern[v]=true;                             // ...and which voice has active patterns
          }
        }
      }
  
      if(msHasPattern[msSelectedVoice][msSelectedPattern]) setBufferPixel(RED, CLEAR_BUTTON); // Pattern contains information, clear button is active (red) 
      setBufferPixel(YELLOW, MUTE_BUTTON);
      if(msMuted[msSelectedVoice]) 
        if (millis()-blinkTimer > blinkTime) setBufferPixel(PINK, MUTE_BUTTON);   // Is the track muted?   
      
      if (msCurrentStep[msSelectedVoice]<16) setBufferPixel(CYAN, SEQ_PAGE_1); // set colors for sequencer pages: cyan-current; blue, active, but not current
      else setBufferPixel(BLUE, SEQ_PAGE_1);
      if (STEPS > 16) {
        if ((msCurrentStep[msSelectedVoice]>=16) && (msCurrentStep[msSelectedVoice]<32)) setBufferPixel(CYAN, SEQ_PAGE_2);
        else setBufferPixel(BLUE, SEQ_PAGE_2);
        if (STEPS > 32) {
          if ((msCurrentStep[msSelectedVoice]>=32) && (msCurrentStep[msSelectedVoice]<48)) setBufferPixel(CYAN, SEQ_PAGE_3);
          else setBufferPixel(BLUE, SEQ_PAGE_3);
          if (STEPS > 48) {
            if (msCurrentStep[msSelectedVoice]>=48) setBufferPixel(CYAN, SEQ_PAGE_4);
            else setBufferPixel(BLUE, SEQ_PAGE_4);
          }
        }
      }
    
      if (!sequencerPaused) {
        setBufferPixel(GREEN, PLAY_BUTTON);                        // indicate on the PLAY button if sequencer is running
        setBufferPixel(YELLOW, RESTART_BUTTON);
      } else setBufferPixel(YELLOW, PLAY_BUTTON);
      
      switch(msDirection[msSelectedVoice]) {                       // show the selected pattern direction
        case FORWARD:
          setBufferPixel(BLUE, DIRECTION_BUTTON);
          break;
        case BACKWARD:
          setBufferPixel(GREEN, DIRECTION_BUTTON);
          break;
        case PINGPONG:
          setBufferPixel(RED, DIRECTION_BUTTON);
          break;
        case RANDOM:
          setBufferPixel(PINK, DIRECTION_BUTTON);
          break;
        }      
    }

    if (millis()-blinkTimer > blinkTime) setBufferPixel(WHITE, page);  // display the page we are at, as a blinking white cursor

    printBuffer(ledmatrixBuffer);
    displayEnc = 0; // stop displaying encoders
  
  } else if (displayEnc != 0) {                                    // normal sequencer mode???
    /* KARG
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
    KARG */

 
  } else {                                                          // shift is not pressed, go through all pages

    if (page==TRACKS_PAGE) {                                        // on tracks page...  
      for (int v=0; v<VOICES; v++) {                                // check which patterns have notes set
        msVoiceHasPattern[v] = false;
        for (int p = 0; p < PATTERNS; p++) {
          msHasPattern[v][p] = false;
          for (int s=0; s<STEPS; s++) {
            if (msStepVelocity[v][p][s]&0x80) {
              msHasPattern[v][p] = true;
              msVoiceHasPattern[v] = true;
            }
          }
        }      
      }
      
      if(probPotChange) patternOffset = map(probPotValue, 0, 127, 0, PATTERNS-1)/4; // KARG: CHANGE THESE TO ENCODERS!      

      for (int col=0; col < 4; col++) {
        for (int row=0; row < 4; row++) {
          if (msHasPattern[col][row+patternOffset*4]) {
            if (msMuted[col]) setPagePixel(PINK, TRACKS_PAGE, row * 4 + col);            
            setPagePixel(YELLOW, TRACKS_PAGE, row * 4 + col);    
          } else setPagePixel(OFF, TRACKS_PAGE, row * 4 + col);
        }

        //if (!sequencerPaused) {                                            // if sequencer is playing
        if ((msCurrentPattern[col]>=patternOffset*4) && (msCurrentPattern[col]<patternOffset*4 + 4)) 
          setPagePixel(BLUE, TRACKS_PAGE, (msCurrentPattern[col]-patternOffset*4) * 4 + col); // display the currently playing track in blue        
 
        if ((col==msSelectedVoice) && (msSelectedPattern>=patternOffset*4) && (msSelectedPattern<patternOffset * 4 + 4)){                                                                     // only display once
          if (stepEditMode) setPagePixel(RED, TRACKS_PAGE, (msSelectedPattern-patternOffset*4) * 4 + msSelectedVoice);  // pattern selected for the GUI is shown in red
          else setPagePixel(WHITE, TRACKS_PAGE, (msSelectedPattern-patternOffset*4) * 4 + msSelectedVoice);
        }
        
        if (millis()-blinkTimer > blinkTime){
          if (msForceNextPattern[col] == 255){                               // and the next track in cyan
            if (msRepeatPattern[col][msCurrentPattern[col]] - msRepeatCounter[col]){
              if ((msCurrentPattern[col]>=patternOffset*4) && (msCurrentPattern[col]<patternOffset*4 + 4))
                setPagePixel(CYAN, TRACKS_PAGE, (msCurrentPattern[col]-patternOffset*4) * 4 + col);      // repeat pattern
            } else if ((msNextPattern[col][msCurrentPattern[col]]>=patternOffset*4) && (msNextPattern[col][msCurrentPattern[col]]<patternOffset*4 + 4))
             setPagePixel(CYAN, TRACKS_PAGE, (msNextPattern[col][msCurrentPattern[col]]-patternOffset*4) * 4 + col);    // next in sequence
          }
          else if ((msForceNextPattern[col]>=patternOffset*4) && (msForceNextPattern[col]<patternOffset*4 + 4))
            setPagePixel(CYAN, TRACKS_PAGE, (msForceNextPattern[col]-patternOffset*4) * 4 + col); // manually changed
        }  
        //}
      }
            
      for (int col=0; col<4; col++)
        for (int row=0; row<4; row++)
          if (buttons[col][row] != buttonsLast[col][row]) {
            if (!buttons[col][row]){                                         // ...a button was pressed      
              //lcdLastTime = 0;                                             // reset LCD timer so that new info is displayed immediately
              if(stepEditMode) {                                             // ... while we are in edit mode
                msSelectedPattern=row+patternOffset*4;
                msSelectedVoice=col;
                if(!buttonPressed) {
                  buttonPressed=true;                                        // mark that a button is pressed
                  buttonPressedNum=msSelectedPattern*4+col;
                } else {                                                     // if one is pressed already, then copy/paste the pattern
                  lcdPrintStr("copy, true");
                  for(int s=0; s<STEPS; s++) {
                    msStepNote[col][row+patternOffset*4][s]=msStepNote[buttonPressedNum%4][buttonPressedNum/4][s];
                    msStepVelocity[col][row+patternOffset*4][s]=msStepVelocity[buttonPressedNum%4][buttonPressedNum/4][s];
                    msStepChance[col][row+patternOffset*4][s]=msStepChance[buttonPressedNum%4][buttonPressedNum/4][s];
                    msStepCC[col][row+patternOffset*4][s]=msStepCC[buttonPressedNum%4][buttonPressedNum/4][s];
                    msStepCCValue[col][row+patternOffset*4][s]=msStepCCValue[buttonPressedNum%4][buttonPressedNum/4][s];
                  }
                  msHasPattern[col][row+patternOffset*4]=msHasPattern[buttonPressedNum%4][buttonPressedNum/4];
                  msVoiceHasPattern[col]=msVoiceHasPattern[buttonPressedNum%4];
                }
              } else {                                                       // ... while we are not in edit mode
                if (sequencerPaused) {                                       // if the sequencer is not playing, directly change to the new pattern
                  msCurrentPattern[col]=row+patternOffset*4;
                } else {
                  msForceNextPattern[col] = row+patternOffset*4;             // otherwise force this pattern on the next change
                }                 
                msSelectedPattern=row+patternOffset*4;
                msSelectedVoice=col;          
              }
              selectedVoice=col;
            } else if((buttonPressed)&&(buttonPressedNum%16==4*row+col)&&(buttonPressedNum/16==patternOffset)) buttonPressed=false; // ...a button was released
          }   
        if (stepEditMode) {                                                  // we are in edit mode, so encoders change numer of repeats and next pattern to play
          if(notePotChange) {
            //lcdLastTime = 0;
            msRepeatPattern[msSelectedVoice][msSelectedPattern] = map(notePotValue, 0, 127, 0, 99);
          }         
          if(octavePotChange) {
            //lcdLastTime = 0;
            msNextPattern[msSelectedVoice][msSelectedPattern] = map(octavePotValue, 0, 127, 0, PATTERNS-1);      
          }
        }
        lcdPrintTwo(msRepeatPattern[msSelectedVoice][msSelectedPattern], msNextPattern[msSelectedVoice][msSelectedPattern]+1);    // show num repeats and next pattern num
    } 
  
    else if (page==CHANNEL_PAGE) {                                           // on channel page...
      lcdPrintInt(msChannel[msSelectedVoice] - 1);                           // ...display the current MIDI channel for this voice on the lcd, while...
      for (int i=0; i<16;i++) setPagePixel(RED, CHANNEL_PAGE, i);            // ...all passible (actually all) buttons are red...
      setPagePixel(PINK, page, msChannel[msSelectedVoice] - 1);              // ...and the current channel button is pink
      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]) 
            if (!buttons[row][col]){                                         // ...a button was pressed, so select a channel
              setPagePixel(OFF, page, msChannel[msSelectedVoice]-1);
              msChannel[msSelectedVoice]=4*col+row+1;
              setPagePixel(PINK, page, msChannel[msSelectedVoice]-1);
              lcdPrintInt(msChannel[msSelectedVoice]);
            }
    }
            
    else if(page==SONG_PAGE) {                                               // on song page
      /* KARG: 
        load/save song, does not need its own page
        the encoder change the song number (which is displayed on the lcd
        one encoder (red illuminated) saves
        the other (green illuminated) loads the song at the given number
      */              
      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]) 
            if (!buttons[row][col]){                                         // ...a button was pressed
              int p=4*col+row;	
              setPagePixel(YELLOW, page, song);                              // new song was selected, turn old button to yellow

              switch(p) {
                case 0:
                case 1:
                case 2:                                                      // load song;
                case 3:
                  song=p;
                  if (sdPresent) loadPatternsSd(song);
                  else loadPatternsEe(song);
                  break;
                case 12:                                                     // save
                  if (sdPresent) savePatternsSd(song);
                  else savePatternsEe(song);
                  break;
              }
              setPagePixel(GREEN, page, song);
            }
    }
    
    else if (page==SCALE_PAGE){                                              // on scale page...
      lcdPrintStr(scalesStrings[msCurrentScale[msSelectedVoice]]);           // ...display the curent scale on the LCD, while...
      for (int i=0;i<12;i++) setPagePixel(PINK, SCALE_PAGE, i);              // ...all possible buttons are lit in pink...
      setPagePixel(CYAN, page, msCurrentScale[msSelectedVoice]);             // ...and the active scale is shown in cyan
      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]){
            if (!buttons[row][col]){                                         // ...a button was pressed, select sequence scale
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
              if(stepEditMode)                                               // fix only for this voice
                for(int p=0;p<PATTERNS;p++)
                  for(int s=0;s<STEPS;s++)
                    while(!noteInScale((msStepNote[msSelectedVoice][p][s]&0x7f),msCurrentScale[msSelectedVoice])) msStepNote[msSelectedVoice][p][s]++;     
              else                                     // fix for all voices
                for(int v=0;v<VOICES;v++)
                  for(int p=0;p<PATTERNS;p++)
                    for(int s=0;s<STEPS;s++)
                      while(!noteInScale((msStepNote[v][p][s]&0x7f),msCurrentScale[v])) msStepNote[v][p][s]++;            
            }
          }
  
    } else if (page==TRANSP_PAGE){                                           // on scale page...
     lcdPrintStr(noteStrings[msRootNote[msSelectedVoice]]);                  // ...display the current selected scale on the screen, while...
     for (int i=0;i<12;i++) 
       switch(i) {
         case 1:
         case 3:
         case 6:
         case 8:
         case 10:
           setPagePixel(CYAN, TRANSP_PAGE, i);                               // ...all possible minor root note buttons are cyan...
           break;
         default:
           setPagePixel(BLUE, TRANSP_PAGE, i);                               // ...the major root note buttons are blue...
           break;
       }
     setPagePixel(YELLOW, page, msRootNote[msSelectedVoice]);                // ...and the actual root note is yellow
       for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]) 
            if (!buttons[row][col]){                                         // ...a button was pressed, select sequence root note
              if(col==3){                                                    // change octave
                if(row==0)//down
                  for(int p=0;p<4;p++)
                    for(int s=0;s<32;s++)
                      if(((msStepNote[msSelectedVoice][p][s]&0x7f)/12)>0) msStepNote[msSelectedVoice][p][s]-=12;
                if(row==1)//up
                  for(int p=0;p<4;p++)
                    for(int s=0;s<32;s++)
                      if(((msStepNote[msSelectedVoice][p][s]&0x7f)/12)<9) msStepNote[msSelectedVoice][p][s]+=12;			
              } else {
                setPagePixel(OFF, page, msRootNote[msSelectedVoice]);
                if(stepEditMode) msRootNote[msSelectedVoice]=constrain(4*col+row,0,11);
                else for(int i=0; i<4; i++) msRootNote[i]=constrain(4*col+row,0,11);
                //setPagePixel(YELLOW, page, msRootNote[msSelectedVoice]);
                if(sequencerPaused) playPreviewNote(msRootNote[msSelectedVoice]+48, 127, msChannel[msSelectedVoice]);   
              }
	    }		  
    } 
  
    else if (page==DURATION_PAGE) {                                          // on duration page...
      lcdPrintInt(16/msStepLength[msSelectedVoice]);                         // ...display the current duration on the lcd, while...
      for (int i=0;i<4;i++) setPagePixel(YELLOW, DURATION_PAGE, i);          // ...all possible buttons are yellow...
      if (msStepLength[msSelectedVoice] == 1) {                              // ...and the actual duration is green
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
            if (!buttons[row][col]){                                         // ...a button was pressed, select sequence length
 
              if(msStepLength[msSelectedVoice]==1) setPagePixel(OFF, page, 0);
              else if(msStepLength[msSelectedVoice]==2) setPagePixel(OFF, page, 1);
              else if(msStepLength[msSelectedVoice]==4) setPagePixel(OFF, page, 2);
              else if(msStepLength[msSelectedVoice]==8) setPagePixel(OFF, page, 3);
            
              if(4*col+row==0)  msStepLength[msSelectedVoice]=1;
              else if(4*col+row==1) msStepLength[msSelectedVoice]=2;
              else if(4*col+row==2) msStepLength[msSelectedVoice]=4;
              else if(4*col+row==3) msStepLength[msSelectedVoice]=8;
             
              if(msStepLength[msSelectedVoice]==1) setPagePixel(GREEN, page, 0);
              else if(msStepLength[msSelectedVoice]==2) setPagePixel(GREEN, page, 1);
              else if(msStepLength[msSelectedVoice]==4) setPagePixel(GREEN, page, 2);
              else if(msStepLength[msSelectedVoice]==8) setPagePixel(GREEN, page, 3);
            }
    }
  
    else if ((page==SEQ_PAGE_1)||(page==SEQ_PAGE_2)||(page==SEQ_PAGE_3)||(page==SEQ_PAGE_4)){ // on sequence pages...
      static byte lastPotChange = NOTE;                                      // -> used to decide which value to display on screen 
      if(stepEditMode){                                                      // ...with edit mode enabled...
        if(octavePotChange){                                                 // ...octave pot readings changed  
          lastPotChange = OCTAVE;
          int octaveValue=map(octavePotValue,0,127,0,9);      
          int currentOctave = (msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f) / 12;
          int currentNote = (msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f) % 12;
          if (currentOctave==octaveValue) {                                  // give control to knob?
            if((sequencerPaused)&&(!octavePotTakeover)) 
              playPreviewNote(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), msStepVelocity[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f, msChannel[msSelectedVoice]);
            octavePotTakeover=true;
          }
          if (octavePotTakeover) {
            if (currentOctave!=octaveValue) {
              MIDI.sendNoteOff(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), 0, msChannel[selectedVoice]);
              usbMIDI.sendNoteOff(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), 0, msChannel[selectedVoice]);
              byte tmpNote=currentNote+octaveValue*12;                       // calculate the note value
              msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]=tmpNote|0x80;  // maintain highest bit
              if(sequencerPaused)
                playPreviewNote(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), msStepVelocity[msSelectedVoice][msSelectedPattern][selectedStep]&0x7F, msChannel[msSelectedVoice]);    
            }
          } else lcdPrintInt(currentOctave);
        }
      
        if(notePotChange){                                                   // ... note pot readings changed
          lastPotChange = NOTE;
          int noteValue=map(notePotValue,0,127,0,msScaleLength[msCurrentScale[msSelectedVoice]]);  //-1
          noteValue=constrain(noteValue,0,msScaleLength[msCurrentScale[msSelectedVoice]]-1);
          int currentOctave = (msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f) / 12;
          int currentNote = (msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f) % 12;
          if (currentNote==scaleNotes[msCurrentScale[msSelectedVoice]][noteValue]) { // give control to knob
            if((sequencerPaused)&&(!notePotTakeover))
              playPreviewNote(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), msStepVelocity[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f, msChannel[msSelectedVoice]);
            notePotTakeover=true;
          }
          if (notePotTakeover) {
            if (currentNote!=scaleNotes[msCurrentScale[msSelectedVoice]][noteValue]) {
              MIDI.sendNoteOff(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), 0, msChannel[selectedVoice]);
              usbMIDI.sendNoteOff(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), 0, msChannel[selectedVoice]);
              byte tmpNote=scaleNotes[msCurrentScale[msSelectedVoice]][noteValue]+currentOctave*12;
              msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]=tmpNote|0x80;
              if(sequencerPaused)
                playPreviewNote(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), msStepVelocity[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f, msChannel[msSelectedVoice]);
            }
          }
        }
    
        if(velocityPotChange){                                               // ... velocity pot readings changed
          lastPotChange = VELOCITY;
          int currentVel = msStepVelocity[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f;
          if(currentVel==velocityPotValue) velocityPotTakeover=true;
            if(velocityPotTakeover){
              if(currentVel!=velocityPotValue){
                MIDI.sendNoteOff(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), 0, msChannel[selectedVoice]);
                usbMIDI.sendNoteOff(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), 0, msChannel[selectedVoice]);
                if (velocityPotValue) msStepVelocity[msSelectedVoice][msSelectedPattern][selectedStep]=velocityPotValue|0x80;
                else msStepVelocity[msSelectedVoice][msSelectedPattern][selectedStep]=0;
                if(sequencerPaused) playPreviewNote(msRootNote[msSelectedVoice]+(msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f), msStepVelocity[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f, msChannel[msSelectedVoice]);  
              }
            }
        }
  
        if(probPotChange){                                                 // ... probalbility pot readings changed
          lastPotChange = PROPABILITY;
          int currentProb = msStepChance[msSelectedVoice][msSelectedPattern][selectedStep];
          if(currentProb==probPotValue) probPotTakeover=true;
            if(probPotTakeover){
              if(currentProb!=probPotValue){
                msStepChance[msSelectedVoice][msSelectedPattern][selectedStep]=probPotValue;
              }
            }
        }

        if ((lastPotChange == OCTAVE) | (lastPotChange == NOTE)) {         // display the last changed value (note+oct / vel / prob) on the lcd
          int currentNote = (msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f) % 12;
          int currentOctave = (msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f) / 12;
          lcdPrintStr(noteStrings[scaleNotes[msCurrentScale[msSelectedVoice]][currentNote]]);
          lcdPrintDigit(currentOctave, 3);          
        } else if (lastPotChange == VELOCITY) { 
          lcdPrintInt(msStepVelocity[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f);
        } else if (lastPotChange == PROPABILITY) {
          lcdPrintInt(map(msStepChance[msSelectedVoice][msSelectedPattern][selectedStep],0,127,0,100));
        }     
      }

      byte stepOffset = 0;
      byte stepMax = 16;
      
      if (page==SEQ_PAGE_2){                        // on the sequence pages ...
        stepOffset = 16;                            // set some variables for the loops
        stepMax = 32;
      } else if (page==SEQ_PAGE_3) {
        stepOffset = 32;
        stepMax = 48;
      } else if (page==SEQ_PAGE_4) {
        stepOffset = 48;
        stepMax = 64;
      } 
        
      for (int i = 0; i < STEPS; i++) {
        if ((stepEditMode) && (selectedStep == i) && (i >= stepOffset) && (i < stepMax)) {
          setPagePixel(RED, page, i % 16);
        } else if ((msStepVelocity[msSelectedVoice][msSelectedPattern][i]&0x80) && (i >= stepOffset) && (i < stepMax)) {    // note is active?
          setPagePixel(GREEN, page, i % 16);
        } else if ((msStepNote[msSelectedVoice][msSelectedPattern][i]&0x80) && (i >= stepOffset) && (i < stepMax)) {        // Legato?
          setPagePixel(YELLOW, page, i % 16);
        } else if (i < 16) {
          setPagePixel(OFF, page, i % 16);
        }
      }
      if ((msCurrentStep[msSelectedVoice] >= stepOffset) && (msCurrentStep[msSelectedVoice] < stepMax)) {
        if (!sequencerPaused) setPagePixel(BLUE, page, msCurrentStep[msSelectedVoice] % msLength[msSelectedVoice] % 16); //currentStep%midiSeqOutLength[selectedVoice]%16
        else if (millis()-blinkTimer > blinkTime) setPagePixel(BLUE, page, msCurrentStep[msSelectedVoice] % msLength[msSelectedVoice] % 16); //currentStep%midiSeqOutLength[selectedVoice]%16
      }
      for (int row=0; row<4; row++)
        for (int col=0; col<4; col++)
          if (buttons[row][col] != buttonsLast[row][col]){ 
            if (!buttons[row][col]){                               // ...a button was pressed, enter pattern
              if(!stepEditMode){                                   // in normal SEQUENCER mode (non-EDIT mode)
                if (!(msStepVelocity[msSelectedVoice][msSelectedPattern][4*col+row+stepOffset]&0x80)){   // step was inactive
                  if (!msStepVelocity[msSelectedVoice][msSelectedPattern][4*col+row+stepOffset]){        // no velocity has been set so far, use the pot
                    if (velocityPotValue) msStepVelocity[msSelectedVoice][msSelectedPattern][4*col+row+stepOffset]=velocityPotValue;              
                    lcdPrintInt(velocityPotValue);              
                  }else{                                                                                 // otherwise just display the curretn note
                    int currentOctave = (msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f) / 12;
                    int currentNote = (msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]&0x7f) % 12;
                    lcdPrintStr(noteStrings[scaleNotes[msCurrentScale[msSelectedVoice]][currentNote]]);
                    lcdPrintDigit(currentOctave, 3);          
                  }
                }
                msStepVelocity[msSelectedVoice][msSelectedPattern][4*col+row+stepOffset]=msStepVelocity[msSelectedVoice][msSelectedPattern][4*col+row+stepOffset]^0x80; // toggle state bit 
            }else{                                               // in EDIT mode...
                if(!buttonPressed){                                // single button was pressed, i.e. note was triggered
                  buttonPressed=true;
                  buttonPressedNum=4*col+row+stepOffset;
                }else{                                             // button was pressed, while another one was held
                  if(4*col+row+stepOffset-buttonPressedNum>0)      // second button is "higher" than first
                    for(int i=buttonPressedNum+1; i<=4*col+row+stepOffset; i++) msStepNote[msSelectedVoice][msSelectedPattern][i]=msStepNote[msSelectedVoice][msSelectedPattern][i]^0x80; // XOR
                  else                                             // second button was "lower" than first -> restart at bottom
                    for(int i=4*col+row; i<=buttonPressedNum-1; i++) msStepNote[msSelectedVoice][msSelectedPattern][i]=(msStepNote[msSelectedVoice][msSelectedPattern][i]^0x80);
                }
              }
              selectedStep=4*col+row+stepOffset;
              octavePotTakeover=false;
              notePotTakeover=false;
              velocityPotTakeover=false;
              probPotTakeover=false;
              //lcdPrintStr(noteStrings[(msRootNote[msSelectedVoice]+msStepNote[msSelectedVoice][msSelectedPattern][selectedStep]) % 12]);
            } else if((buttonPressed)&&(buttonPressedNum%16==4*col+row)) buttonPressed=false;   // ...button was released  
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
// function to print buffer on the button pad
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
// function to print buffer on the button pad
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
    if(lcdShowDigit[i]){
      if((i==2)&&(decimal)) shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, (lcdDigit[i])+0x80);  
      else shiftOut(LCD_DATA_PIN, LCD_CLOCK_PIN, MSBFIRST, (lcdDigit[i]));  
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
  if ((!lcdLastTime) | ((lcdLastTime) && (millis() - lcdLastTime > lcdDelay))) {
    lcdLastTime = 0;

    if(num>9){
      if(num>99){
        if(num>999){
          lcdShowDigit[0]=true;
          lcdDigit[0]=sevencodes[num/1000];
          lcdShowDigit[1]=true;
          lcdDigit[1]=sevencodes[num/100%10];
          lcdShowDigit[2]=true;
          lcdDigit[2]=sevencodes[num/10%10];
          lcdShowDigit[3]=true;
          lcdDigit[3]=sevencodes[num%10];
        }else{
          lcdShowDigit[0]=false;
          lcdShowDigit[1]=true;
          lcdDigit[1]=sevencodes[num/100];
          lcdShowDigit[2]=true;
          lcdDigit[2]=sevencodes[num/10%10];
          lcdShowDigit[3]=true;
          lcdDigit[3]=sevencodes[num%10];
        }
      }else{
        lcdShowDigit[0]=false;
        lcdShowDigit[1]=false;
        lcdShowDigit[2]=true;
        lcdDigit[2]=sevencodes[num/10];
        lcdShowDigit[3]=true;
        lcdDigit[3]=sevencodes[num%10];
      }
    }else{
      lcdShowDigit[0]=false;
      lcdShowDigit[1]=false;
      lcdShowDigit[2]=false;
      lcdShowDigit[3]=true;
      lcdDigit[3]=sevencodes[num];
    }
    decimal=false;
  }
}

//######################################################################################
void lcdPrintDigit(byte num, byte digit){
  if ((!lcdLastTime) | ((lcdLastTime) && (millis() - lcdLastTime > lcdDelay))) {
    lcdLastTime = 0;
    lcdShowDigit[digit]=true;
    lcdDigit[digit]=sevencodes[num];
  }
}

//######################################################################################
void lcdPrintTwo(byte num1, byte num2){
  if ((!lcdLastTime) | ((lcdLastTime) && (millis() - lcdLastTime > lcdDelay))) {
    lcdLastTime = 0;
    
    while (num1 > 99) num1 -= 100;
    if (num1 > 9) {
      lcdShowDigit[0]=true;
      lcdDigit[0]=sevencodes[num1/10];
    } else lcdDigit[0] = false;
    lcdShowDigit[1]=true;
    lcdDigit[1]=sevencodes[num1%10];
    
    while (num2 > 99) num1 -= 100;
    if (num2 > 9) {
      lcdShowDigit[2]=true;
      lcdDigit[2]=sevencodes[num2/10];
    } else lcdDigit[2] = false;  
    lcdShowDigit[3]=true;
    lcdDigit[3]=sevencodes[num2%10];
  }
}

//######################################################################################
void lcdPrintFloat(float num){
  if ((!lcdLastTime) | ((lcdLastTime) && (millis() - lcdLastTime > lcdDelay))) {
    lcdLastTime = 0;
    if(num>0){
      if(num>9){
        if(num>99){
          lcdShowDigit[0]=true;
          lcdDigit[0]=sevencodes[(int)num/100%10];
          lcdShowDigit[1]=true;
          lcdDigit[1]=sevencodes[(int)num/10%10];
          lcdShowDigit[2]=true;
          lcdDigit[2]=sevencodes[(int)num%10];
          lcdShowDigit[3]=true;
          lcdDigit[3]=sevencodes[(int)(num*10)%10];
        }else{
          lcdShowDigit[0]=false;
          lcdShowDigit[1]=true;
          lcdDigit[1]=sevencodes[(int)num/10%10];
          lcdShowDigit[2]=true;
          lcdDigit[2]=sevencodes[(int)num%10];
          lcdShowDigit[3]=true;
          lcdDigit[3]=sevencodes[(int)(num*10)%10];
        }
      }else{
        lcdShowDigit[0]=false;
        lcdShowDigit[1]=false;
        lcdShowDigit[2]=true;
        lcdDigit[2]=sevencodes[(int)num%10];
        lcdShowDigit[3]=true;
        lcdDigit[3]=sevencodes[(int)(num*10)%10];
      }
    }else{
      lcdShowDigit[0]=false;
      lcdShowDigit[1]=false;
      lcdShowDigit[2]=true;
      lcdDigit[2]=sevencodes[0];
      lcdShowDigit[3]=true;
      lcdDigit[3]=sevencodes[(int)(num*10)%10];
    }
    decimal=true;
  }
}

//######################################################################################
void lcdPrintStr(char const* str, boolean important){
  if ((important) || (!lcdLastTime) || (lcdLastTime && (millis() - lcdLastTime > lcdDelay))){
    
    lcdShowDigit[0]=false;
    lcdShowDigit[1]=false;
    lcdShowDigit[2]=false;
    lcdShowDigit[3]=false;
  
    for (int i = 0; i<4; i++){
      if(str[i]=='\0') break;
      if(str[i]==' ') lcdDigit[i]=sevencodes[10];
      else if(str[i]=='#') lcdDigit[i]=sevencodes[11];
      else if(str[i]>47 && str[i]<58) lcdDigit[i]=sevencodes[str[i]-'0'];
      else lcdDigit[i]=sevencodes[(int)(str[i]-'a')+12];
      lcdShowDigit[i]=true;
    }
    decimal=false;
    if (important) lcdLastTime = millis();
  }
}

