// Functions:
//
// void Loop()                                         - Main program
//
// #####################################################################################################################
void loop() {    
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
    } else {
      isShift = false;
      //displayEnc=0; // stop displaying encoders
      //loadLedState();
    }
  }

  if ((page == TRACKS_PAGE) || (page == SEQ_PAGE_1) || (page == SEQ_PAGE_2)) { // read edit button
    if ( editButton.update() ) {
      if ( editButton.read() == LOW) {
        stepEditMode = !stepEditMode;
        digitalWrite(ENCX_YLED_PIN, stepEditMode);
        lcdPrintStr("edit");
      }
    }
  }

  if ( seqButton.update() ) {                                      // read sequence button
    if ( seqButton.read() == LOW) {
      if (page == SEQ_PAGE_1) page = SEQ_PAGE_2;
      else page = SEQ_PAGE_1;
      lcdPrintStr("seqc");
    }
  }

  if ( trackButton.update() ) {                                    // read tracks button
    if ( trackButton.read() == LOW) {
      page = TRACKS_PAGE;
      lcdPrintStr("trac");
      for (int i = 0; i < 4; i++) {
        msChannelHasPattern[i] = false;

        for (int k = 0; k < 4; k++) {
          msHasPattern[i][k] = false;
          for (int j = 0; j < steps; j++) {
            if (msStepState[i][k][j]) {
              msHasPattern[i][k] = true;
              msChannelHasPattern[i] = true;
            }
          }
        }
      }

      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          if (msHasPattern[i][j]) setPagePixel(YELLOW, 5, j * 4 + i);
          else setPagePixel(0, 5, j * 4 + i);
        }
        if (msMuted[i]) setPagePixel(PINK, 5, msSelectedPattern[i] * 4 + i);
        else setPagePixel(GREEN, 5, msSelectedPattern[i] * 4 + i);

        if (stepEditMode) setPagePixel(RED, 5, msSelectedSequence);
        else setPagePixel(WHITE, 5, msSelectedSequence);
      }
    }
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
}

