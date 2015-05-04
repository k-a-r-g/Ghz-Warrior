// Functions:
//
// void internalSequencerStep()                        - ...
// void checkStep(int i)                               - ...
//
//######################################################################################
void internalSequencerStep(){
  if(!sequencerPaused){
    MIDI.sendRealTime(Clock);
    // usbMIDI.sendRealTimeClock();
    
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


//######################################################################################

void checkStep(int i) {
  if (!sequencerPaused) {  
    if ((msLastPlayed[i])&&(!msStepLegato[i][msSelectedPattern[i]][msCurrentStep[i]])) { //currentStep%midiSeqOutLength[i]
      //midiNoteOnOff(false, i-1+7*16);
      usbMIDI.sendNoteOff(msLastPlayedNote[i], 0, msChannel[i]);
      if(sequencerMidiOut) MIDI.sendNoteOff(msLastPlayedNote[i], 0, msChannel[i]);
      msLastPlayed[i]=false;
      msLastTie[i]=false;
    } else if(msStepLegato[i][msSelectedPattern[i]][msCurrentStep[i]]) msLastTie[i]=true;
    
    if (((msStepState[i][msSelectedPattern[i]][msCurrentStep[i]])&&(msLastTie[i]))){
      usbMIDI.sendNoteOff(msLastPlayedNote[i], 0, msChannel[i]);
      MIDI.sendNoteOff(msLastPlayedNote[i], 0, msChannel[i]);
      msLastPlayed[i]=false;
      msLastTie[i]=false;
    }

    if ((msStepState[i][msSelectedPattern[i]][msCurrentStep[i]])&&(!msMuted[i])) { //currentStep%midiSeqOutLength[i]
      int prob = random(0, 127);
      if(prob<msStepChance[i][msSelectedPattern[i]][msCurrentStep[i]]){
        usbMIDI.sendNoteOn(msRootNote[i]+msStepNote[i][msSelectedPattern[i]][msCurrentStep[i]], msStepVelocity[i][msSelectedPattern[i]][msCurrentStep[i]], msChannel[i]);
        if(sequencerMidiOut) MIDI.sendNoteOn(msRootNote[i]+msStepNote[i][msSelectedPattern[i]][msCurrentStep[i]], msStepVelocity[i][msSelectedPattern[i]][msCurrentStep[i]], msChannel[i]); //currentStep%midiSeqOutLength[i]
          msLastPlayed[i]=true;
	  msLastTie[i]=false;
          msLastPlayedNote[i]=msRootNote[i]+msStepNote[i][msSelectedPattern[i]][msCurrentStep[i]]; //currentStep%midiSeqOutLength[i]
        }
      }
    } 
}
