// Functions:
//
// void internalSequencerStep()                        - the internal clock routines (triggered by clockTimer timer)
// void advanceStep()                                  - process the current step (triggered by the internal and external clock)
// void checkStep(int v)                               - checks if notes be turned on/off in voice v the current step (triggered by advanceStep )
//
//######################################################################################


void internalSequencerStep(){
  if(!sequencerPaused){
    MIDI.sendRealTime(Clock);
    usbMIDI.sendRealTime(Clock);
    
    clockCounter++; 
    
    executeStep();
    if (clockCounter == 48) { //24
      clockCounter = 0;
    }
  }
}



//######################################################################################
void executeStep() {
  
  for(int v=0; v<VOICES;v++){
    if (clockCounter%(6*msStepLength[v]) == 0) {                                                   // 6
                                                                                                   // we first determine which is the next step to play...
      if(msDirection[v]==FORWARD){                                                                 // forward
        msCurrentStep[v]++;
        if (msCurrentStep[v]>=msLength[v]) msCurrentStep[v]=0;
      }

      if(msDirection[v]==BACKWARD){                                                                // backwards
        msCurrentStep[v]--;
        if (msCurrentStep[v]<0) msCurrentStep[v]=msLength[v]-1;

      }else if(msDirection[v]==PINGPONG){                                                          // ping pong
        if(msDirAscending[v]){                                                                     // ping pong: forward
          msCurrentStep[v]++;
          if (msCurrentStep[v]>=msLength[v]){
            msDirAscending[v]=false;
            msCurrentStep[v]-=2;
          }
        }else{                                                                                     // ping pong: backwards
          msCurrentStep[v]--;
          if (msCurrentStep[v]<0){
            msDirAscending[v]=true;
            msCurrentStep[v]+=2;
          }
        }
      }
      if(msDirection[v]==RANDOM) msCurrentStep[v]=random(0,msLength[v]);                           // random      
    
      checkStep(v);
      
      msStepCounter[v]++;                                                                          // ...and then check if the pattern is completed
      if (msStepCounter[v]>=msLength[v]){                                                          // enough steps of this pattern haven processed
        if (msRepeatPattern[v][msCurrentPattern[v]]>msRepeatCounter[v]){
          msRepeatCounter[v]++;                                                                    // should we repeat this pattern?
        }else{
          msRepeatCounter[v]=0;
          msCurrentPattern[v]=msNextPattern[v][msCurrentPattern[v]];                               // if not, then jump to next pattern
          if (msForceNextPattern[v]<255){
            msCurrentPattern[v] = msForceNextPattern[v];                                           // if a pattern was manually set in the GUI, use this
            msForceNextPattern[v]=255;                                                             // inactivate 
          }
        }
        
        msStepCounter[v] = 0;
      }    
    }
  }
}


//######################################################################################
void checkStep(int v) {
  if (!sequencerPaused) {  
    if ((msLastPlayed[v])&&(!(msStepNote[v][msCurrentPattern[v]][msCurrentStep[v]]&0x80))) {           // currentStep%midiSeqOutLength[v]
      //midiNoteOnOff(false, v-1+7*16);
      usbMIDI.sendNoteOff(msLastPlayedNote[v], 0, msChannel[v]);
      if(sequencerMidiOut) MIDI.sendNoteOff(msLastPlayedNote[v], 0, msChannel[v]);
      msLastPlayed[v]=false;
      msLastTie[v]=false;
    } else if(msStepNote[v][msCurrentPattern[v]][msCurrentStep[v]]&0x80) msLastTie[v]=true;
    
    if ((msStepVelocity[v][msCurrentPattern[v]][msCurrentStep[v]]&0x80)&&(msLastTie[v])){
      usbMIDI.sendNoteOff(msLastPlayedNote[v], 0, msChannel[v]);
      MIDI.sendNoteOff(msLastPlayedNote[v], 0, msChannel[v]);
      msLastPlayed[v]=false;
      msLastTie[v]=false;
    }

    if ((msStepVelocity[v][msCurrentPattern[v]][msCurrentStep[v]]&0x80)&&(!msMuted[v])) {                 // currentStep%midiSeqOutLength[v]
      int prob = random(0, 127);
      if(prob<msStepChance[v][msCurrentPattern[v]][msCurrentStep[v]]){
        usbMIDI.sendNoteOn(msRootNote[v]+(msStepNote[v][msCurrentPattern[v]][msCurrentStep[v]]&0x7f), msStepVelocity[v][msCurrentPattern[v]][msCurrentStep[v]]&0x7f, msChannel[v]);
        if(sequencerMidiOut) MIDI.sendNoteOn(msRootNote[v]+(msStepNote[v][msCurrentPattern[v]][msCurrentStep[v]]&0x7f), msStepVelocity[v][msCurrentPattern[v]][msCurrentStep[v]]&0x7f, msChannel[v]); //currentStep%midiSeqOutLength[v]
          msLastPlayed[v]=true;
	  msLastTie[v]=false;
          msLastPlayedNote[v]=msRootNote[v]+(msStepNote[v][msCurrentPattern[v]][msCurrentStep[v]]&0x7f); // currentStep%midiSeqOutLength[v]
        }
      }
    } 
}
