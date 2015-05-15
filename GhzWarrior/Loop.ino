// Functions:
//
// void Loop()                                         - Main program
//
// #####################################################################################################################
void loop(){
  if ((bpmChange) && (!externalClock)) {                           // BPM were changes, restart the clock
    //noInterrupts();
    if ((millis() - bpmChangeTime) > 25) {
      clockTimer.end();
      clockTimer.begin(internalSequencerStep, sequencerInterval);
      bpmChange = false;
    }
    //interrupts();
  }

  if (shiftButton.update()) {                                      // read shift button
    if (shiftButton.read() == LOW) {
      isShift = true;
      //displayEnc=0; // stop displaying encoders
      //saveLedState();
    } else {                                                       // shift got released
      isShift = false;
      //displayEnc=0; // stop displaying encoders
      //loadLedState();
    }
  }

  if ((page == TRACKS_PAGE) || (page == SEQ_PAGE_1) || (page == SEQ_PAGE_2) || (page == SEQ_PAGE_3) || (page == SEQ_PAGE_4)) { // read edit button
    if ( editButton.update() ) {
      if (editButton.read() == LOW) {
        stepEditMode = !stepEditMode;
        digitalWrite(ENCX_YLED_PIN, stepEditMode); 
        if (stepEditMode) lcdPrintStr("edit", true);
        else switch(page) { 
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
          case TRACKS_PAGE:               
            lcdPrintStr("trck", true);
            break;                
        }
      }
    }
  }

  if ( seqButton.update() ) {                                      // read sequence button
    if ( seqButton.read() == LOW) {
      if ((page>=SEQ_PAGE_1) && (page<=SEQ_PAGE_4)){  
        page++;                                                    // when pressed, cycle through sequencer pages
        if (page>SEQ_PAGE_4) page = SEQ_PAGE_1;
      }else page = SEQ_PAGE_1;
      switch (page) {
        case SEQ_PAGE_1:
          lcdPrintStr("sen1");
          break;
        case SEQ_PAGE_2:
          lcdPrintStr("sen2");
          break;
        case SEQ_PAGE_3:
          lcdPrintStr("sen3");
          break;
        case SEQ_PAGE_4:
          lcdPrintStr("sen4");
          break;
      }        
    }
  }

  if ( trackButton.update() )                                      // read tracks button
    if (trackButton.read() == LOW) {
      page = TRACKS_PAGE;
      lcdPrintStr("trac");
    }
  
  
  readEditPots();                                                  // read all potentiometer values
  readButtonpad();                                                 // check if buttons are pressed/released on the pad
  updateUI();                                                      // update the UI


  if (msPlayingPreview) {                                          // check preview notes
    if ((millis() - msPreviewTime) > 50) {
      MIDI.sendNoteOff(msPreviewNote, 0, msPreviewChannel);
      usbMIDI.sendNoteOff(msPreviewNote, 0, msPreviewChannel);
      msPlayingPreview = false;
    }
  }
  //

  // Karg: encBut[page*encodersBanked][0]->read();
  // Karg: encBut[page*encodersBanked][1]->read();

  //encLed[page*encodersBanked][0]->set();
  //encLed[page*encodersBanked][1]->set();

  //readEncoders();

  usbMIDI.read();                                                   // read all the incoming midi messages

  if ((millis()-blinkTimer) > 2*blinkTime) blinkTimer = millis();   // reset blink timer
}

