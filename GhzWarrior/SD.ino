// Functions:
//
// void loadSetupSd()                                  - load the configuration from Sd card
// void saveSetupEe()                                  - save the configuration to Sd card
// void loadPatternsSd(int v)                          - load pattern number v from eeprom 
// void savePatternsSd(int v)                          - save pattern to eeprom slot v
// void int2filename(int numv)                         - converts a 4gidigt integer to a song filename of the format ????.SNG and writes it into the global char array fileName
//
// note:
// in contrast to the eeprom, no limitations apply to pattern saved on the Sd card
//
//######################################################################################
void initSd() {
#ifdef SD_MOD
  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  //if (sd.begin(SD_CARD_SS_PIN, SPI_HALF_SPEED)) sdPresent = true;
#endif
}

//######################################################################################
void loadSetupSd() {
#ifdef SD_MOD
  sd.chdir(); // change current directory to root
  if (!file.open("GhzWar.set")) saveSetupSd(); // if we're in the root and we can't open the file, generate it
  else { // load the settings


    //int address = 0;
    //byte evalue;

    //evalue = EEPROM.read(address);
    //if(evalue == 2) { // load rest of settings
    //  loadPatternsEe(0);// load song 0
    //} else { // set default settings
    //  int address = 0;
    //  byte evalue = 2;
    //  EEPROM.write(address, evalue); // save a settings flag
    //  for(int i=0;i<3;i++) { //save blank patterns to avoid errors on loading
    //    savePatternsEe(i);
    //  }
    //}
    file.close();
  }
#endif
}

//######################################################################################
//save settings to SD
void saveSetupSd(){ 
#ifdef SD_MOD
  int settingsBuffer[10];

  file.open("GhzWar.set", O_RDWR | O_CREAT); // create file if it doesn't exist and open the file for write
  if (file.write(settingsBuffer, 40) != -1) { // note - we are writing 10 4 byte ints from the patch buffer to 40 bytes on the SD
    if (file.sync()) lcdPrintStr("done");
  }
  file.close();
#endif
}

//######################################################################################
//load Patterns from SD
void loadPatternsSd(int v){
#ifdef SD_MOD
  // v was the parameter, so this selects the song:
  
  int address;
  byte sdBuffer[8];
  boolean sdError = false;
  byte evalue;

  int2filename(v);
  if (!file.open(fileName)) lcdPrintStr("err4", true);
  else {
    for (int v = 0; v < VOICES; v++) {                       // voices
      for (int p = 0; p < PATTERNS; p++) {                   // patterns
        for (int s = 0; s < STEPS; s++) {                    // steps    ****  32
          if ((file.read(sdBuffer, 5) != 5) || (sdError)) {
            sdError = true;
            break;
          }
          address = 0;          
          evalue = sdBuffer[address];
          msStepNote[v][p][s] = evalue;                      // note & legato (highest bit)
          address++;
          evalue = sdBuffer[address];                        // velocity & state (highest bit)
          if (evalue) msStepVelocity[v][p][s] = evalue|0x80;
          else msStepVelocity[v][p][s] = 0; 
          address++;
          evalue = sdBuffer[address];                        // propability
          msStepChance[v][p][s] = evalue;
          address++;
          evalue = sdBuffer[address];                        // CC
          msStepCC[v][p][s] = evalue;
          address++;
          evalue = sdBuffer[address];                        // CC value
          msStepCCValue[v][p][s] = evalue;
        }  
        if ((file.read(sdBuffer, 2) != 2) || (sdError)) {
          sdError = true;
          break;
        }
        address = 0;
        evalue = sdBuffer[address];                          // pattern repeat
        msRepeatPattern[v][p] = evalue;
        address++;        
        evalue = sdBuffer[address];                          // pattern sequence
        msNextPattern[v][p] = evalue;
      }      
      if ((file.read(sdBuffer, 5) != 5) || (sdError)) {
        sdError = true;
        break;
      }
      address = 0;
      evalue = sdBuffer[address];                              
      msChannel[v] = evalue & 0xF;                           // channel
      msCurrentScale[v] = (evalue >> 4) & 0xF;               // scale
      address++;
      evalue = sdBuffer[address];                            // mute status
      msMuted[v]= evalue;
      address++;
      evalue = sdBuffer[address];                              
      msRootNote[v] = evalue & 0xF;                          // transition
      msDirection[v] = (evalue >> 4) & 0x3;                  // direction
      address++;
      evalue = sdBuffer[address];                              
      msLength[v] = evalue;                                  // length of this pattern
      address++;
      evalue = sdBuffer[address];                              
      msStepLength[v] = evalue;                              // length of each step
    }
    
    for (int s=0;s<SLIDES;s++){
      address = 0;
      if ((file.read(sdBuffer, 8) != 8) || (sdError)) {
        sdError = true;
        break;
      }
      slVoice[s] = sdBuffer[address];
      address++;
      slStartBeat[SLIDES] = sdBuffer[address];;
      address++;
      slStartStep[SLIDES] = sdBuffer[address];;              
      address++;
      slStartValue[SLIDES] = sdBuffer[address];;
      address++;
      slStopBeat[SLIDES] = sdBuffer[address];;
      address++;
      slStopStep[SLIDES] = sdBuffer[address];;
      address++;
      slStopValue[SLIDES] = sdBuffer[address];;
      address++;
      slRemainding[SLIDES] = sdBuffer[address];;                  
    }
  }
  file.close();
  if (sdError) lcdPrintStr("err1", true);
  else lcdPrintStr("load", true);
#endif
}

//######################################################################################
void savePatternsSd(int v){                                // v is the song number 
#ifdef SD_MOD
  
  byte sdBuffer[8];  // the sd card uses 512 byte buffers, so anything smaller won't make much sense
  byte evalue;
  int address;
  boolean sdError= false;
  
  int2filename(v);
  if (!file.open(fileName, O_RDWR | O_CREAT)){
    lcdPrintStr("err3", true);
  }else{
    for(int v=0;v<VOICES;v++){                             // voices    
      for(int p=0;p<PATTERNS;p++){                         // patterns
        for(int s=0;s<STEPS;s++){                          // steps        
          address = 0;
          evalue=msStepNote[v][p][s];                      // note + octave, legato
          sdBuffer[address] = evalue;
          address++;
          evalue=msStepVelocity[v][p][s];                  // velocity, state
          sdBuffer[address] = evalue;
          address++;
          evalue=msStepChance[v][p][s];                    // probalility
          sdBuffer[address] = evalue;
          address++;
          evalue=msStepCC[v][p][s];                        // CC number, CC state
          sdBuffer[address] = evalue;
          address++;
          evalue=msStepCCValue[v][p][s];                   // CC value
          sdBuffer[address] = evalue;
          if (!(file.write(sdBuffer, 5) == 5)) sdError = true;
        }
        address = 0;
        evalue=msRepeatPattern[v][p];                      // pattern repeat
        sdBuffer[address] = evalue;
        address++;
        evalue=msNextPattern[v][p];                        // pattern sequence
        sdBuffer[address] = evalue;
        if (!(file.write(sdBuffer, 2) == 2)) sdError = true;
      }
      address = 0;
      evalue = msChannel[v];                               // channel
      evalue += msCurrentScale[v]<<4;                      // scale
      sdBuffer[address] = evalue;
      address++;
      evalue = msMuted[v];                                 // mute status
      sdBuffer[address] = evalue;
      address++;
      evalue = msRootNote[v];                              // transition
      evalue += msDirection[v]<<4;                         // direction
      sdBuffer[address] = evalue;
      address++;
      evalue = msLength[v];                                // pattern length
      sdBuffer[address] = evalue;
      address++;
      evalue = msStepLength[v];                            // step length
      sdBuffer[address] = evalue;
      if (!(file.write(sdBuffer, 5) == 5)) sdError = true;
    }

    for (int s=0;s<SLIDES;s++){
      address = 0;
      sdBuffer[address] = slVoice[s];
      address++;
      sdBuffer[address] = slStartBeat[SLIDES];
      address++;
      sdBuffer[address] = slStartStep[SLIDES];              
      address++;
      sdBuffer[address] = slStartValue[SLIDES];
      address++;
      sdBuffer[address] = slStopBeat[SLIDES];
      address++;
      sdBuffer[address] = slStopStep[SLIDES];
      address++;
      sdBuffer[address] = slStopValue[SLIDES];
      address++;
      sdBuffer[address] = slRemainding[SLIDES];                  
      if (!(file.write(sdBuffer, 8) == 8)) sdError = true;
    }    
    if ((file.sync() && (!sdError))) lcdPrintStr("save", true);
    else lcdPrintStr("err2", true);
  }
  file.close();
#endif
}


//######################################################################################
void int2filename(int num){
#ifdef SD_MOD
  char lcdDigit[5];

  memset(lcdDigit,0,sizeof(lcdDigit));
  memset(fileName,0,sizeof(fileName));

  byte i = 1;
  if(num>9){
    if(num>99){
      if(num>999){
        lcdDigit[0]=(num/1000)+'0';
        lcdDigit[1]=(num/100%10)+'0';
        lcdDigit[2]=(num/10%10)+'0';
        lcdDigit[3]=(num%10)+'0';
        i = 4;
      }
      else{
        lcdDigit[0]=(num/100)+'0';
        lcdDigit[1]=(num/10%10)+'0';
        lcdDigit[2]=(num%10)+'0';
        i = 3;
      }
    }else{
      lcdDigit[0]=(num/10)+'0';
      lcdDigit[1]=(num%10)+'0';
      i = 2;
    } 
  } else lcdDigit[0]=(num%10)+'0';
  
  
  for (int f = 0; f < i; f++) fileName[f] = lcdDigit [f];
  strncpy(fileName+i, ".sng", 4);
#endif
}

