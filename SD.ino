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
  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  if (!sd.begin(SD_CARD_SS_PIN, SPI_HALF_SPEED)) lcdPrintStr("nosd");
  else sdPresent = true;
}

//######################################################################################
void loadSetupSd() {

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
}

//######################################################################################
//save settings to SD
void saveSetupSd() {
  int settingsBuffer[10];

  file.open("GhzWar.set", O_RDWR | O_CREAT); // create file if it doesn't exist and open the file for write
  if (file.write(settingsBuffer, 40) != -1) { // note - we are writing 10 4 byte ints from the patch buffer to 40 bytes on the SD
    if (file.sync()) lcdPrintStr("done");
  }
  file.close();
}

//######################################################################################
//load Patterns from SD
void loadPatternsSd(int v) {
  // v was the parameter, so this selects the song:

  byte patternBuffer[1548];
  int address = 0;
  byte evalue;

  int2filename(v);
  file.open(fileName);
  if (file.read(patternBuffer, 1548) == 1548) { 
    lcdPrintStr("load");
  
    for (int v = 0; v < VOICES; v++) {                       // voices
      
      for (int p = 0; p < PATTERNS; p++) {                   // patterns
        for (int s = 0; s < STEPS; s++) {                    // steps
          evalue = patternBuffer[address];
          msStepNote[v][p][s] = evalue & 0xF;                // note
          msStepNote[v][p][s] += ((evalue >> 4) & 0x7) * 12; // octave
          msStepLegato[v][p][s] = (((evalue >> 7) & 1) == 1);// legato
          msStepChance[v][p][s] = 127;
          address++;

          evalue = patternBuffer[address];                   // velocity
          msStepVelocity[v][p][s] = evalue;          
          if (msStepVelocity[v][p][s] > 0) msStepState[v][p][s] = true; 
          else msStepState[v][p][s] = false;                 // state
          address++;
        
          evalue = patternBuffer[address];                   // propability
          msStepChance[v][p][s] = evalue;
          address++;
        }
      }
      
      //track settings
      evalue = patternBuffer[address];
      msChannel[v] = evalue & 0xF;                           // channel
      msCurrentScale[v] = (evalue >> 4) & 0xF;               // scale
      address++;
      evalue = patternBuffer[address];
      msRootNote[v] = evalue & 0xF;                          // transition
      msDirection[v] = (evalue >> 4) & 0x3;                  // direction
      address++;
      evalue = patternBuffer[address];
      msLength[v] = evalue & 0x3F;                           // length
      msStepLength[v] = pow(2, evalue >> 6);                 // step length
      address++;
      msMuted[v] = false;
    }
  }
}

//######################################################################################
void savePatternsSd(int v){                                  //v is the song number
  
  byte patternBuffer[1548];  //652
  int address = 0;
  byte evalue;

  int2filename(v);
  file.open(fileName, O_RDWR | O_CREAT); // create file if it doesn't exist and open the file for write

  for(int v=0;v<VOICES;v++){                                 // voices
    
    for(int p=0;p<PATTERNS;p++){                             // patterns
      for(int s=0;s<STEPS;s++){                              // steps
        evalue=msStepNote[v][p][s]%12;                       // note
        evalue+=constrain((msStepNote[v][p][s]/12),0,7)<<4;  // octave
        if(msStepLegato[v][p][s]) evalue+=1<<7;              // legato
        patternBuffer[address] = evalue;
        address++;
        evalue=msStepVelocity[v][p][s];                      // velocity
        patternBuffer[address] = evalue;
        address++;
        evalue=msStepChance[v][p][s];                        // probalility
        patternBuffer[address] = evalue;
        address++;
      }
    }
    
    //track settings
    evalue = msChannel[v];                                   // channel
    evalue += msCurrentScale[v]<<4;                          // scale
    patternBuffer[address] = evalue;
    address++;
    evalue = msRootNote[v];                                  // transition
    evalue += msDirection[v]<<4;                             // direction
    patternBuffer[address] = evalue;
    address++;
    //length step length
    evalue = msLength[v];                                    // length
    switch(msStepLength[v]){                                 // step length WHAT IS THE DIFFERENCE?
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
    patternBuffer[address] = evalue;
    address++;
  }
  
  if (file.write(patternBuffer, 1548) == 1548) {
    if (file.sync()) lcdPrintStr("save");
  }
  file.close();
}

//######################################################################################
void int2filename(int num){
  char digit[5];

  memset(digit,0,sizeof(digit));
  memset(fileName,0,sizeof(fileName));

  byte i = 1;
  if(num>9){
    if(num>99){
      if(num>999){
        digit[0]=(num/1000)+'0';
        digit[1]=(num/100%10)+'0';
        digit[2]=(num/10%10)+'0';
        digit[3]=(num%10)+'0';
        i = 4;
      }
      else{
        digit[0]=(num/100)+'0';
        digit[1]=(num/10%10)+'0';
        digit[2]=(num%10)+'0';
        i = 3;
      }
    }else{
      digit[0]=(num/10)+'0';
      digit[1]=(num%10)+'0';
      i = 2;
    } 
  } else digit[0]=(num%10)+'0';
  
  
  for (int f = 0; f < i; f++) fileName[f] = digit [f];
  strncpy(fileName+i, ".sng", 4);

}

