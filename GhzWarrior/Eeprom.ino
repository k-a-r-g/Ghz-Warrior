// Functions:
//
// void loadSetupEe()                                  - load the configuration from eeprom
// void saveSetupEe()                                  - save the configuration to eeprom
// void loadPatternsEe(int v)                          - load pattern number v from eeprom 
// void savePatternsEe(int v)                          - save pattern to eeprom slot v
//
// note:
// up to 3 songs can be stored on the device, each song consists of the 4 clips for each of the 4 tracks 
// and can be saved or loaded on the fly as the sequencer is running. due to limited memory capacity the step 
// probability of each step does not get saved, also the velocity resolution of each step gets scaled down to 3-bits. 
//
//######################################################################################
void loadSetupEe(){
  int address = 0;
  byte evalue;
  
  evalue = EEPROM.read(address);
  if(evalue == 2) { // load rest of settings
    loadPatternsEe(0);// load song 0
  } else { // set default settings
    int address = 0;
    byte evalue = 2;
    EEPROM.write(address, evalue); // save a settings flag
    for(int i=0;i<3;i++) { //save blank patterns to avoid errors on loading
      savePatternsEe(i);
    }
  }
}

//######################################################################################
void loadPatternsEe(int v){
  int address = 1 + v*652; // song offset
  byte evalue;
  
  for(int i=0;i<4;i++) {      // tracks
    for(int p=0;p<4;p++) {    //patterns
      for (int s=0;s<4;s++) { // velocity
        evalue = EEPROM.read(address);
        msStepVelocity[i][p][s*8]=((evalue)&0x3)*42;
        msStepVelocity[i][p][s*8+1]=((evalue>>(1*2))&0x3)*42;
        msStepVelocity[i][p][s*8+2]=((evalue>>(2*2))&0x3)*42;
        msStepVelocity[i][p][s*8+3]=((evalue>>(3*2))&0x3)*42;
        address++;
				
        evalue = EEPROM.read(address);
        msStepVelocity[i][p][s*8+4]=((evalue)&0x3)*42;
        msStepVelocity[i][p][s*8+5]=((evalue>>(1*2))&0x3)*42;
        msStepVelocity[i][p][s*8+6]=((evalue>>(2*2))&0x3)*42;
        msStepVelocity[i][p][s*8+7]=((evalue>>(3*2))&0x3)*42;
        address++;
      }
      for(int s=0;s<32;s++) { //steps, due to EEPROM limitations, 32 is the max
        evalue = EEPROM.read(address);
        //if(msStepVelocity[i][p][s]>0) msStepState[i][p][s] = true; // state
        //else msStepState[i][p][s] = false;
        	
        msStepNote[i][p][s]=evalue&0xF; // note
        msStepNote[i][p][s]+=((evalue>>4)&0x7)*12; // octave
        //KARG:: msStepLegato[i][p][s]=(((evalue>>7)&1)==1);
        msStepChance[i][p][s]=127;
        address++;
      }
    }
    //track settings
    //scale channel
    evalue = EEPROM.read(address);
    msChannel[i]=evalue&0xF;
    msCurrentScale[i]=(evalue>>4)&0xF;
    address++;
    // transition dir
    evalue = EEPROM.read(address);
    msRootNote[i]=evalue&0xF;
    msDirection[i]=(evalue>>4)&0x3;
    address++;
    //length step length
    evalue = EEPROM.read(address);
    msLength[i]=evalue&0x3F;
    msStepLength[i]=pow(2,evalue>>6);
    address++;
    msMuted[i]=false;  
  }    
}

//######################################################################################
void savePatternsEe(int v){
  int address = 1 + v*652; // song offset
  byte evalue;
  
  for(int i=0;i<4;i++) { // tracks
    for(int p=0;p<4;p++) { //patterns
      for (int s=0;s<4;s++) { // velocity			
        evalue=msStepVelocity[i][p][s*8]/42;
        evalue+=(msStepVelocity[i][p][s*8+1]/42)<<(1*2);
        evalue+=(msStepVelocity[i][p][s*8+2]/42)<<(2*2);
        evalue+=(msStepVelocity[i][p][s*8+3]/42)<<(3*2);
        EEPROM.write(address, evalue);
        address++;
				
        evalue=msStepVelocity[i][p][s*8+4]/42;
        evalue+=(msStepVelocity[i][p][s*8+5]/42)<<(1*2);
        evalue+=(msStepVelocity[i][p][s*8+6]/42)<<(2*2);
        evalue+=(msStepVelocity[i][p][s*8+7]/42)<<(3*2);
        EEPROM.write(address, evalue);
        address++;
      }
      for(int s=0;s<32;s++){ //steps
        evalue=msStepNote[i][p][s]%12; // note
        evalue+=constrain((msStepNote[i][p][s]/12),0,7)<<4; // octave
				
        //KARG: if(msStepLegato[i][p][s]) evalue+=1<<7;
        EEPROM.write(address, evalue);
        address++;
      }
    }
    //track settings
    //scale channel
    evalue = msChannel[i];
    evalue += msCurrentScale[i]<<4;
    EEPROM.write(address, evalue);
    address++;
    // transition dir
    evalue = msRootNote[i];
    evalue += msDirection[i]<<4;
    EEPROM.write(address, evalue);
    address++;
    //length step length
    evalue = msLength[i];
    switch(msStepLength[i]){
      case 1:
        break;
      case 2:
        evalue +=1<<6;
        break;
      case 4:
        evalue +=2<<6;
        break;
      case 8:
        evalue +=3<<6;
        break;
    }
    EEPROM.write(address, evalue);
    address++;
  }
}

