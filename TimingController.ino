//This sketch is for Arduino Micro in timing controller hand paddle.  Assumed peripheral hardware:
//Serial1 -wired to hand paddle controller to communicate with LCD and keypad
//D2/D3 - I2C wired to DB15 connector for future expansion
//D11/D4/A5 - wired through opto to DB15 as input.  High means input grounded.
//D5/D6/D9/D10/D12/A4 - wired through darlington array to DB15 as output.  High means drive ground on output
//D7/D8 - wired to RS232 level shift and out RJ11 for future expansion using software serial
//D13 - wired to speaker drive transistor through capacitor.  Use PWM output.
//SCK(D15) - left side button, use input_pullup.  0 means pressed.
//MISO(D14) - right side button, use input_pullup.  0 means pressed.
//A3 - left side button led.  low means turn led on.
//A2 - right side button led.  low means turn led on.
//A1 - not used
//A0 - wired through 33 ohm to DB15 as analog input for future expansion.

//Assumed external wiring:
//Input1/Opto1/DB15-12 Keyswitch.
//Input2/Opto2/DB15-4 Main firing button
//Input3/Opto3/DB15-5 Not used
//Output1/Dar1/DB15-9 HSI1/a  (channel 0 in sequence var)
//Output2/Dar2/DB15-10 HSI2/b  (channel 1)
//Output3/Dar3/DB15-1 firing button LED  (channel 2)
//Output4/Dar4/DB15-2 Not used  (channel 3)
//Output5/Dar5/DB15-11 Solenoid-ignited  (channel 4)
//Output6/Dar6/DB15-3 Solenoid-not ignited  (channel 5)

#include <EEPROM.h>


byte HSI1 = 0; //State of HSI. 0 for off, 1 for on.
byte HSI2 = 0; //State of HSI. 0 for off, 1 for on.
byte TriggerLED = 0; //state of led on main trigger button.  0 for off
byte ExtraOut = 0; //State of extra output.  0 for off.
byte Key = 0; //state of key switches.  1 for engaged
byte Button = 0; //State of main firing button
byte ExtraIn = 0;  //state of the extra input
byte Armed = 0; //State of arming.  1 for armed.(button pressed)
byte testmode = 0; //1 for in test mode
byte DisplayType = 0; //0 for basic display, 1 for more information
byte sequencenum = 0; //which sequence was loaded
byte edited = 0; //flag if sequence edited from what was loaded.  1 for edited.
byte numsteps = 0; //number of steps in current sequence
byte Input1Last; //last state of input 1 pin   //Prior copies for debounce and edge detection
byte Input1Last2; //last state of input 1 pin
byte Input2Last; //last state of input 2 pin
byte Input2Last2; //last state of input 2 pin
byte Input3Last; //last state of input 3 pin
byte Input3Last2; //last state of input 3 pin
byte LeftButtonLast; //last state of the left button
byte LeftButtonLast2; //last state of the left button
byte RightButtonLast; //last state of the right button
byte RightButtonLast2; //last state of the right button
byte RightButtonLEDState; //state of right button led.  0 = off; 1 = on; 2 = flash; 3 = fast flash
byte LeftButtonLEDState; //state of left button led.  0 = off; 1 = on; 2 = flash
int ButtonFlashCounter; //counter to support flashing.

#include "TimingController.h" //Constant definitions and defines

unsigned int sequence[maxsteps][3];  //[step number][value]
  //value 0 - number of ms to wait before doing this step
  //value 1 - channel to activate.  0-5. ff-end of sequence.  fe-illegal
  //value 2 - state to set channel. 0-off, 1-on

#define masterpasscodelength 8
const int MasterPasscode[masterpasscodelength] = {ZEROkey, ZEROkey, ZEROkey, ZEROkey, ZEROkey, ZEROkey, ZEROkey, ZEROkey };
int passcodelength;
int Passcode[15];  //maximum 15 digits for passcode

#include "LCDFunctions.h" //functions supporting displaying on the LCD

//return the length in steps of the current sequence
//returns 0 if no end flag found.
byte sequencelength()
{
  for (byte j = 0; j < maxsteps; j++)
  {
    if (sequence[j][1] == 0xff)
      return (j+1);
  }
  return 0;
}

//load a saved sequence
//takes in which sequence to load
//returns the number of steps
byte loadsequence(byte number)
{
  int addressbase = number;
  addressbase = addressbase << 7; //multiply by 128, assuming 128byte pages for the saves
  int tempadr;
  int tempdata;
  for (int i = 0; i < (maxsteps); i++)
  {
    for (int j = 0; j < 3; j++)
    {
      tempadr = addressbase + (i*6) + (j*2); //calc the byte count; 
      tempdata = EEPROM.read(tempadr);
      tempdata = tempdata << 8; //shift to high bite.
      tempdata = tempdata + EEPROM.read(tempadr+1);  //combine low byte
      sequence[i][j] = tempdata;
    }
  }
  edited = 0;
  sequencenum = number;
  numsteps = sequencelength();
  return numsteps;
}

//save the sequence
//takes in which sequence to load
void savesequence(byte number)
{
  int addressbase = number;
  addressbase = addressbase << 7; //multiply by 128, assuming 128byte pages for the saves
  int tempadr;
  int tempdata;
  byte tempbyte;
  for (int i = 0; i < (maxsteps); i++)
  {
    for (int j = 0; j < 3; j++)
    {
      tempadr = addressbase + (i*6) + (j*2); //calc the byte count; 
      tempdata = sequence[i][j];
      tempbyte = tempdata >> 8; //get the high byte to save
      EEPROM.write(tempadr, tempbyte);
      tempbyte = tempdata & 0x00FF; //mask off the low byte
      EEPROM.write((tempadr+1), tempbyte);
    }
  }
  edited = 0;
  sequencenum = number;
}

//Prompts, and does a soft reset if confirmed
//Restarts bootloader + program from beginning but does not reset the peripherals and registers
void Reset()
{
  ClearLCD();
  Serial1.write("Reset contr");
  delay(50);
  Serial1.write("oller?\r\n");
  delay(50);
  Serial1.write("ENT-> Yes  ESC-> No \r\n");
  Serial1.write(0x07);  //send bell character.
  Serial1.write(0x07);  //send bell character.
  Serial1.write(0x07);  //send bell character.
//Clear off the input buffer
  while (((Serial1.read()) != -1));  
  for (int i = 0; ((i < 10000) && (Serial1.available() == 0)); i++)
    delay(1);   //wait here until get a key, or timeout at 10sec
  if (Serial1.read() == ENTERkey)
  {
    Serial1.write(0x07);  //send bell character.
    Serial1.write("Resetting");
    delay(100);
    asm volatile ("  jmp 0");
  }
  else
  {
    Serial1.write("Reset canceled!");
    delay(2000);
  }
  FullUpdate();     //refresh screen
}

void setup() {
//  Serial.begin(9600);  //start usb serial for debugging.
  Serial1.begin(9600, SERIAL_7E1); //start serial port to LCD/buttons.  Note 9600 7E1 mode.
  pinMode(Input1_Pin, INPUT);
  pinMode(Input2_Pin, INPUT);
  pinMode(Input3_Pin, INPUT);
  digitalWrite(Output1_Pin, LOW); //drive low to turn off output
  digitalWrite(Output2_Pin, LOW); //drive low to turn off output
  digitalWrite(Output3_Pin, LOW); //drive low to turn off output
  digitalWrite(Output4_Pin, LOW); //drive low to turn off output
  digitalWrite(Output5_Pin, LOW); //drive low to turn off output
  digitalWrite(Output6_Pin, LOW); //drive low to turn off output
  pinMode(Output1_Pin, OUTPUT);
  pinMode(Output2_Pin, OUTPUT);
  pinMode(Output3_Pin, OUTPUT);
  pinMode(Output4_Pin, OUTPUT);
  pinMode(Output5_Pin, OUTPUT);
  pinMode(Output6_Pin, OUTPUT);
  digitalWrite(SoftSerialTX_Pin, HIGH);
  pinMode(SoftSerialTX_Pin, OUTPUT);
  pinMode(SoftSerialRX_Pin, INPUT);
  digitalWrite(Speaker_Pin, LOW);
  pinMode(Speaker_Pin, OUTPUT);
  pinMode(LeftButton_Pin, INPUT_PULLUP);
  pinMode(RightButton_Pin, INPUT_PULLUP);
  digitalWrite(LeftLED_Pin, HIGH);
  pinMode(LeftLED_Pin, OUTPUT);
  digitalWrite(RightLED_Pin, HIGH);
  pinMode(RightLED_Pin, OUTPUT);

//Init variables to starting state.
  Input1Last = digitalRead(Input1_Pin);
  Input1Last2 = Input1Last; 
  Input2Last = digitalRead(Input2_Pin);
  Input2Last2 = Input2Last; 
  Input3Last = digitalRead(Input3_Pin);
  Input3Last2 = Input3Last; 
  LeftButtonLast = digitalRead(LeftButton_Pin);
  LeftButtonLast2 = LeftButtonLast;
  RightButtonLast = digitalRead(RightButton_Pin);
  RightButtonLast2 = RightButtonLast;

  HSI1 = 0; //State of HSI. 0 for off, 1 for on.
  HSI2 = 0; //State of HSI. 0 for off, 1 for on.
  TriggerLED = 0; //state of led on main trigger button.  0 for off
  ExtraOut = 0; //State of extra output.  0 for off.
  Key = Input1Last2; //state of key switches.  1 for engaged
  Button = Input2Last2; //State of main firing button
  ExtraIn = Input3Last2;  //state of the extra input
  Armed = 0; //State of arming.  1 for armed.(button pressed)
  testmode = 0; //1 for in test mode
  DisplayType = 0; //0 for basic display, 1 for more information
  sequencenum = 0; //which sequence was loaded
  edited = 0; //flag if sequence edited from what was loaded.  1 for edited.
  numsteps = 0; //number of steps in current sequence
  ButtonFlashCounter = 0;
  RightButtonLEDState = 0;
  LeftButtonLEDState = 1;
  
//Stuff for future expansion:
  pinMode(SCL_Pin, INPUT_PULLUP); //SCL pin, probably not needed; wire.begin probably does this anyway.
  pinMode(SDA_Pin, INPUT_PULLUP); //SDA pin, probably not needed; wire.begin probably does this anyway.
  digitalWrite(SCL_Pin, HIGH); //SCL pin, probably not needed.; wire.begin probably does this anyway.
  digitalWrite(SDA_Pin, HIGH); //SDA pin, probably not needed.; wire.begin probably does this anyway.
  pinMode(AnalogIn_Pin, INPUT); //analog input pin from DB15

  loadsequence(0); //load sequence 0 as default.
  
//load passcode data from EEPROM
  passcodelength = EEPROM.read(1007);
  for (int i = 0; i < 16; i++)
    Passcode[i] = EEPROM.read(1008+i);
    
  delay(2000); //delay to allow paddle processor to boot

  LockScreen();  //start locked

  ClearLCD();
  FullUpdate();
  
  while (((Serial1.read()) != -1));  //Clear off the input buffer
}


//maintain the state/flash of the left/right button leds
//call in main loop
void UpdateButtonLEDs()
{
  ButtonFlashCounter++;
  if (ButtonFlashCounter == FlashSpeed)
  {
    ButtonFlashCounter = 0;
    if (RightButtonLEDState >= 2)
      digitalWrite(RightLED_Pin, !(digitalRead(RightLED_Pin)));
    if (LeftButtonLEDState >= 2)
      digitalWrite(LeftLED_Pin, !(digitalRead(LeftLED_Pin)));
  }
  if (ButtonFlashCounter == (FlashSpeed/2))
  {
    if (RightButtonLEDState >= 3)
      digitalWrite(RightLED_Pin, !(digitalRead(RightLED_Pin)));
    if (LeftButtonLEDState >= 3)
      digitalWrite(LeftLED_Pin, !(digitalRead(LeftLED_Pin)));
  }
  if (RightButtonLEDState == 0)
    digitalWrite(RightLED_Pin, HIGH);
  if (LeftButtonLEDState == 0)
    digitalWrite(LeftLED_Pin, HIGH);
  if (RightButtonLEDState == 1)
    digitalWrite(RightLED_Pin, LOW);
  if (LeftButtonLEDState == 1)
    digitalWrite(LeftLED_Pin, LOW);  
}


//prompt for passcode.  enter key to enter it; any other key accepted as a digit.
//returns 1 for correct, 0 for bad.
byte EnterPasscode()
{
  while (((Serial1.read()) != -1));  //Clear off the input buffer
//  Serial1.write("\r");
  Serial1.write("Pass: ");
  int receivedbutton = -1;
  int receivecount = 0;
  int receivedsequence[15];
  while (receivedbutton != ENTERkey)
  {
    receivedbutton = Serial1.read();
    if ((receivedbutton != -1) && (receivedbutton != ENTERkey))
    {
      Serial1.write('*');
      receivedsequence[receivecount] = receivedbutton;
      receivecount++;
    }
    if (receivecount == 15)
      break;
    delay(1);
  }
  byte correct = 0;
  if (receivecount == passcodelength)
  {
    for (int i = 0; i < receivecount; i++)
    {
      if (receivedsequence[i] != Passcode[i])
        break;
      if (i == (receivecount - 1))
        correct = 1;
    }
  }
  if (receivecount == masterpasscodelength)
  {
    for (int i = 0; i < masterpasscodelength; i++)
    {
      if (receivedsequence[i] != MasterPasscode[i])
        break;
      if (i == (masterpasscodelength - 1))
        correct = 1;
    }
  }
  return correct;
}

//Lock and unlock with passcode.  No actions if locked.
//Stays in function while locked.
void LockScreen()
{
  while (((Serial1.read()) != -1));  //Clear off the input buffer
  RightButtonLEDState = 2;
  LeftButtonLEDState = 2;
  while (1)
  {
    Serial1.write("\n\n\n\r    ");
    delay(50);
    Serial1.write("** LOCKED **\r\n");
    while (Serial1.available() == 0)
    {
      UpdateButtonLEDs();
      delay(1);   //wait here until get a key
    }
    if (Serial1.read() == BLANKkey)
      if (EnterPasscode() == 1)
      {
        Serial1.write("\r\n\r\n");
        delay(100);
        RightButtonLEDState = 0;
        LeftButtonLEDState = 0;
        return;
      }
  }
}

//Change passcode (saved one only, no change to master)
void ChangePasscode()
{
  Serial1.write("\r\n\n\n\n");
  delay(50);
  if (EnterPasscode() != 1)
  {
    Serial1.write("\r\n** Not Changed **\r\n");
    delay(2000);
    return;
  }
  while (((Serial1.read()) != -1));  //Clear off the input buffer
  Serial1.write("\r\nNew Pass: ");
  int receivedbutton = -1;
  int receivecount = 0;
  int receivedsequence[15];
  while (receivedbutton != ENTERkey)
  {
    receivedbutton = Serial1.read();
    if ((receivedbutton != -1) && (receivedbutton != ENTERkey))
    {
      Serial1.write('*');
      receivedsequence[receivecount] = receivedbutton;
      receivecount++;
    }
    if (receivecount == 15)
      break;
    delay(1);
  }
  Serial1.write("\r\nRe-Enter: ");
  receivedbutton = -1;
  int receive2count = 0;
  int received2sequence[15];
  while (receivedbutton != ENTERkey)
  {
    receivedbutton = Serial1.read();
    if ((receivedbutton != -1) && (receivedbutton != ENTERkey))
    {
      Serial1.write('*');
      received2sequence[receive2count] = receivedbutton;
      receive2count++;
    }
    if (receivecount == 15)
      break;
    delay(1);
  }
  //check if the two entries match
  byte correct = 0;
  if (receivecount == receive2count)
  {
    for (int i = 0; i < receivecount; i++)
    {
      if (receivedsequence[i] != received2sequence[i])
        break;
      if (i == (receivecount - 1))
        correct = 1;
    }
  }
  if (correct != 1)
  {
    Serial1.write("\r\n** Not Changed **\r\n");
    delay(2000);
    return;
  }
  passcodelength = receivecount;
  EEPROM.write(1007, (byte) passcodelength);
  for (int i = 0; i < receivecount; i++)
  {
    Passcode[i] = receivedsequence[i];
    EEPROM.write(1008+i, Passcode[i]);
  }
  Serial1.write("\r\n** Changed **\r\n");
  delay(2000);
}

//prompts for confirmation, then saves sequence
void savemenu(byte number)
{
  Serial1.write("\r\n\r\n"); //return to column 1
  Serial1.write("Save to ");
  Serial1.print(number);
  Serial1.write("?\r\n");
  Serial1.write("ENT-");
  delay(50);
  Serial1.write("> Yes  ");
  delay(150);
  Serial1.write("ESC-> No \r\n");
  delay(100);
//Clear off the input buffer
  while (((Serial1.read()) != -1));  
  for (int i = 0; ((i < 10000) && (Serial1.available() == 0)); i++)
    delay(1);   //wait here until get a key, or timeout at 10sec
  if (Serial1.read() == ENTERkey)
  {
    savesequence(number-1); //save the sequence
    Serial1.write("Save complete!\r\n");
    Serial1.write(0x07);  //send bell character.
    delay(2000);
  }
  else
  {
    Serial1.write("Save canceled!\r\n");
    delay(2000);
  }
  FullUpdate();     //refresh screen
}

//prompts for confirmation, then loads sequence
void loadmenu(byte number)
{
  Serial1.write("\r\n\r\n"); //return to column 1
  Serial1.write("Load from: ");
  Serial1.print(number);
  Serial1.write("?\r\n");
  delay(100);
  Serial1.write("ENT-> Yes  ");
  delay(100);
  Serial1.write("ESC-> No \r\n");
//Clear off the input buffer
  while (((Serial1.read()) != -1));  
  for (int i = 0; ((i < 10000) && (Serial1.available() == 0)); i++)
    delay(1);   //wait here until get a key, or timeout at 10sec
  if (Serial1.read() == ENTERkey)
  {
    loadsequence(number-1); //load the sequence
    Serial1.write("Load complete!\r\n");
    Serial1.write(0x07);  //send bell character.
    delay(2000);
  }
  else
  {
    Serial1.write("Load canceled!\r\n");
    delay(2000);
  }
  FullUpdate();     //refresh screen
}

//Helper for UpdateSequence
//returns 1, 0 if pressed.  returns 0x2 if enter pressed.  returns -1 if  escape pressed
int CheckForBinInput()
{
  while (1)
  {
    while (((Serial1.read()) != -1));  //Clear off the input buffer
    while (Serial1.available() == 0)
      delay(1);   //wait here until get a key
    int ReceivedByte = Serial1.read();
    switch (ReceivedByte)
    {
      case ONEkey:
        return 1;
        break;  
      case ZEROkey:
        return 0;
        break;
      case ENTERkey:
        return 2;
        break;
      case ESCAPEkey:
        return -1;
        break;
      default:
        break;
    }
  }
}

//Helper for UpdateSequence
//returns 0-5 if 1-6 pressed.  returns 0x10 if enter pressed.  returns -1 if  escape pressed
int CheckForChInput()
{
  while (1)
  {
    while (((Serial1.read()) != -1));  //Clear off the input buffer
    while (Serial1.available() == 0)
      delay(1);   //wait here until get a key
    int ReceivedByte = Serial1.read();
    switch (ReceivedByte)
    {
      case ONEkey:
        return 0;
        break;  
      case TWOkey:
        return 1;
        break;
      case THREEkey:
        return 2;
        break;
      case FOURkey:
        return 3;
        break;
      case FIVEkey:
        return 4;
        break;
      case SIXkey:
        return 5;
        break;
      case ENTERkey:
        return 0x10;
        break;
      case ESCAPEkey:
        return -1;
        break;
      default:
        break;
    }
  }
}

//Helper for UpdateSequence
//returns 0-9 if pressed.  returns 0x10 if enter pressed.  returns -1 if  escape pressed
int CheckForDigInput()
{
  while (1)
  {
    while (((Serial1.read()) != -1));  //Clear off the input buffer
    while (Serial1.available() == 0)
      delay(1);   //wait here until get a key
    int ReceivedByte = Serial1.read();
    switch (ReceivedByte)
    {
      case ZEROkey:
        return 0;
        break;  
      case ONEkey:
        return 1;
        break;  
      case TWOkey:
        return 2;
        break;
      case THREEkey:
        return 3;
        break;
      case FOURkey:
        return 4;
        break;
      case FIVEkey:
        return 5;
        break;
      case SIXkey:
        return 6;
        break;
      case SEVENkey:
        return 7;
        break;  
      case EIGHTkey:
        return 8;
        break;  
      case NINEkey:
        return 9;
        break;  
      case ENTERkey:
        return 0x10;
        break;
      case ESCAPEkey:
        return -1;
        break;
      default:
        break;
    }
  }
}

//interactive prompts to edit the sequence.
// 12345678901234567890
//----------------------
//|ENTER-Cont. ESC-Exit|
//|Step: 01  |
//|End(1-Yes 0-No): Yes|
//|Initial delay: 00000|
//|Channel: 1          |
//|Turn(1-On 0-Off):Off|
//----------------------
//print prompt, current value, if number pressed, \r, reprint prompt, take new value
  //value 0 - number of ms to wait before doing this step
  //value 1 - channel to activate.  0-5. ff-end of sequence
  //value 2 - state to set channel. 0-off, 1-on
void UpdateSequence()
{
  ClearLCD();

//update edited flag if appropriate

//Print instructions, and step number
  Serial1.write("ENTER-Cont. ");
  delay(80);
  Serial1.write("ESC-Exit\r\n");
  for (byte i = 0; i < maxsteps; i++)
  {
    Serial1.write("Step: ");
    delay(80);
    Serial1.print(i+1);
//Query for end flag
    delay(80);
    Serial1.write("\r\nEnd(1-Yes 0-No): ");
    if (sequence[i][1] == 0xff)
      Serial1.write("Yes");
    else
      Serial1.write("No ");
    switch (CheckForBinInput())
    {
      case 1:
        Serial1.write(0x8); //backspace
        Serial1.write(0x8);
        Serial1.write(0x8);
        Serial1.write("Yes");
        sequence[i][1] = 0xff; //mark as end
        sequence[i][0] = 0;  //set time to 0
        sequence[i][2] = 0;
        edited = 1; //mark as the sequence has been edited
        break;
      case 0:
        Serial1.write(0x8); //backspace
        Serial1.write(0x8);
        Serial1.write(0x8);
        Serial1.write("No ");
        if (sequence[i][1] == 0xff)
          sequence[i][1] = 0xfe; //mark as not end
        edited = 1; //mark as the sequence has been edited
        break;
      case 2:  //pressed enter, leave data as-is, go on.
        break;
      case -1:  //pressed escape, bail out of update
        return;
        break;
    }
//Query for initial time delay
    if (sequence[i][1] != 0xff) //only ask these if not already flagged as end.
    {
      Serial1.write("\r\nInitial delay: ");
      Serial1.print(sequence[i][0]);
      unsigned int AccumulatedValue = 0;
      int value = CheckForDigInput();
      while ((value != 0x10) && (value != -1))
      {
        if (AccumulatedValue > 6552) //check if about to break bounds of unsigned int
          return;  //lame handling of error by bailing out.
        AccumulatedValue *= 10;  //add in the new digit
        AccumulatedValue += value;
        Serial1.write("\rInitial delay: "); //reprint entire line
        Serial1.print(AccumulatedValue);
        value = CheckForDigInput();  //get new button press
        if (value == 0x10)  //if enter, save it off before leaving
        {
          sequence[i][0] = AccumulatedValue;
          edited = 1; //mark as the sequence has been edited
        }
      }
      if (value == -1)  //escape, bail out without any other updates.
        return;

//Query for channel
      Serial1.write("\r\nChannel: ");
      if (sequence[i][1] != 0xfe)  //only print the current channel if it is legal
        Serial1.print(sequence[i][1]);
      value = CheckForChInput();
      switch (value)
      {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
          Serial1.write("\rChannel: "); //overwrite line
          Serial1.print(value+1);
          sequence[i][1] = value;
          edited = 1; //mark as the sequence has been edited
          break;
        case 0x10: //pressed enter, leave data as-is ,go on.
          break;
        case -1: //pressed escape, bail out of update
          return;
          break;
      }
      
//Query for on/off
      Serial1.write("\r\nTurn(1-On 0-Off):");
      if (sequence[i][2] == 0)
        Serial1.write("Off");
      else
        Serial1.write("On ");
      switch (CheckForBinInput())
      {
        case 1:
          Serial1.write(0x8); //backspace
          Serial1.write(0x8);
          Serial1.write(0x8);
          Serial1.write("On ");
          sequence[i][2] = 1;
          edited = 1; //mark as the sequence has been edited
          break;
        case 0:
          Serial1.write(0x8); //backspace
          Serial1.write(0x8);
          Serial1.write(0x8);
          Serial1.write("Off");
          sequence[i][2] = 0;
          edited = 1; //mark as the sequence has been edited
          break;
        case 2:  //pressed enter, leave data as-is, go on.
          break;
        case -1:  //pressed escape, bail out of update
          return;
          break;
      }
    }
    Serial1.write("\r\n");
    delay(100);

  }
  Serial1.write("Max Steps Reached!\r\n");
  Serial1.write("Press a key to exit");
  while (Serial1.available() == 0)
    delay(1);   //wait here until get a key    
}

//this runs the sequence in memory
//If end of sequence flag is set, does not execute that step, or any afterwards.
  //value 0 - number of ms to wait before doing this step
  //value 1 - channel to activate.  0-5. ff-end of sequence
  //value 2 - state to set channel. 0-off, 1-on
void RunSequence()
{
  byte length = sequencelength() - 1;  //determine in advance how many steps to run.
  for (byte i = 0; i < length; i++)
  {
    unsigned int delaytime = sequence[i][0];
//    delay(sequence[i][0]); //start by waiting the initial delay.  Will trust on values
    while (delaytime > 100)   //breakup the delay into 100ms or smaller chunks to check for button release.
    {
      delay(100);
      delaytime -= 100;
      if (digitalRead(LeftButton_Pin) != 0)  //confirm arming is still held
        return;
    }
    if (delaytime > 0)
      delay(delaytime);
    if (digitalRead(LeftButton_Pin) != 0)  //confirm arming is still held
      return;
    int pinnumber = 0xff;
    switch (sequence[i][1])
    {
      case 0:
        pinnumber = Output1_Pin;
        break;
      case 1:
        pinnumber = Output2_Pin;
        break;
      case 2:
        pinnumber = Output3_Pin;
        break;
      case 3:
        pinnumber = Output4_Pin;
        break;
      case 4:
        pinnumber = Output5_Pin;
        break;
      case 5:
        pinnumber = Output6_Pin;
        break;
//can add in other channels here, such as speaker beeps, or other future use hardware/I2C
      default:
        pinnumber = 0xff;
        break;
    }
    if (pinnumber != 0xff) //only if valid entry found in switch above
    {
      if (sequence[i][2] == 1)
        digitalWrite(pinnumber, 1);
      else
        digitalWrite(pinnumber, 0);
    }
  }
}


void loop() {
 
//Check and process action triggers

  if ((Input2Last == 1) && (digitalRead(Input2_Pin) == 1) && (Input2Last2 == 0))
  { //input2 went active.  Main firing button
    Button = Input2Last;
    if ((Armed == 1) && (Key == 1))  //if key and arm, short pause, check again, then run sequence.  
    {
      Serial1.write("\r\n\n");
      delay(50);
      Serial1.write("    ** Fi");
      Serial1.write("ring **\n\n");
      delay(LaunchDelay);  //safety delay, wait and then check the three inputs again before going.
      Serial1.write(0x07);  //send bell character.
      if ((digitalRead(LeftButton_Pin) == 0) && (digitalRead(Input1_Pin) == 1) && (digitalRead(Input2_Pin) == 1))
      {
        RunSequence();
        digitalWrite(Output5_Pin, LOW);
        digitalWrite(Output6_Pin, LOW);
        delay(200);
        Serial1.write("\rWaiting for release");
        byte temp = 0;
        while ((digitalRead(Input2_Pin) == 0) || (temp = 0))
        {
          temp = digitalRead(Input2_Pin);
          delay(500);
          }
      }

    }
    FullUpdate();
  }
  if ((Input2Last == 0) && (digitalRead(Input2_Pin) == 0) && (Input2Last2 == 1))
  { //input2 went inactive)    .  Main firing button
    Button = Input2Last;
    FullUpdate();
  }
  if ((Input1Last == 1) && (digitalRead(Input1_Pin) == 1) && (Input1Last2 == 0))
  { //input1 went active.  Keyswitch
    Key = Input1Last;
    TriggerLED = 1;
    digitalWrite(Output3_Pin, TriggerLED);
    FullUpdate();
  }
  if ((Input1Last == 0) && (digitalRead(Input1_Pin) == 0) && (Input1Last2 == 1))
  { //input1 went inactive  Keyswitch
    Key = Input1Last;
    TriggerLED = 0;
    digitalWrite(Output3_Pin, TriggerLED);
    FullUpdate();
  }
  if ((Input3Last == 1) && (digitalRead(Input3_Pin) == 1) && (Input3Last2 == 0))
  { //input3 went active)    .  Extra input
    ExtraIn = Input3Last;
    FullUpdate();
  }
  if ((Input3Last == 0) && (digitalRead(Input3_Pin) == 0) && (Input3Last2 == 1))
  { //input3 went inactive)    .  Extra input
    ExtraIn = Input3Last;
    FullUpdate();
  }
  if ((RightButtonLast == 0) && (digitalRead(RightButton_Pin) == 0) && (RightButtonLast2 == 1))
  { //Right button pressed)    
    if (testmode == 1)
    {
      digitalWrite(Output6_Pin, 1);
      FullUpdate();
    }
    else  //if arm, short pause, check again, then run sequence
    {
      if ((Armed == 1))  //if arm, short pause, check again, then run sequence.  
      {
        Serial1.write("\r\n\n");
        delay(50);
        Serial1.write("    ** Fi");
        Serial1.write("ring **\n\n");
        delay(LaunchDelay);  //safety delay, wait and then check the two inputs again before going.
        Serial1.write(0x07);  //send bell character.
        if ((digitalRead(LeftButton_Pin) == 0) && (digitalRead(RightButton_Pin) == 0))
        {
          RunSequence();
          digitalWrite(Output5_Pin, LOW);
          digitalWrite(Output6_Pin, LOW);
          delay(200);
          Serial1.write("\rWaiting for release");
          byte temp = 0;
          while ((digitalRead(RightButton_Pin) == 0) || (temp = 0))
          {
            temp = digitalRead(RightButton_Pin);
            delay(500);
          }
        }
      }
      FullUpdate();
    }
  }
  if ((RightButtonLast == 1) && (digitalRead(RightButton_Pin) == 1) && (RightButtonLast2 == 0))
  { //Right button released)    
    if (testmode == 1)
    {
      digitalWrite(Output6_Pin, 0);
      FullUpdate();
    }
  }
  if ((LeftButtonLast == 0) && (digitalRead(LeftButton_Pin) == 0) && (LeftButtonLast2 == 1))
  { //Left button pressed)    
    if (testmode == 1)
      digitalWrite(Output5_Pin, 1);
    Armed = 1;
    RightButtonLEDState = 3;
    FullUpdate();
  }
  if ((LeftButtonLast == 1) && (digitalRead(LeftButton_Pin) == 1) && (LeftButtonLast2 == 0))
  { //Left button released)    
    digitalWrite(Output5_Pin, 0);
    if (testmode != 1)
      digitalWrite(Output6_Pin, 0); //if not in test mode, shut off both when arming released
    Armed = 0;
    RightButtonLEDState = 0;
    FullUpdate();
  }
  int ReceivedByte = Serial1.read();
  switch (ReceivedByte)
  {
    case STATUSkey:  //toggle display type, and refresh screen
      if (DisplayType == 0)
        DisplayType = 1;
      else
        DisplayType = 0;  
      ClearLCD();
      delay(50);
      FullUpdate(); //(re)print the main status window
      break;
    case CHANGEkey:
      LeftButtonLEDState = 0;
      UpdateSequence();
      numsteps = sequencelength();
      LeftButtonLEDState = 1;
      FullUpdate();
      break;
    case Akey:  //toggle HSI1/HSIa state
      if (HSI1 == 0)
        HSI1 = 1;
      else
        HSI1 = 0;  
      digitalWrite(Output1_Pin, HSI1);
      FullUpdate();
      break;
    case Bkey:  //toggle HSI2/HSIb state
      if (HSI2 == 0)
        HSI2 = 1;
      else
        HSI2 = 0;  
      digitalWrite(Output2_Pin, HSI2);
      FullUpdate();
      break;
    case Ckey:  //toggle trigger button led
      if (TriggerLED == 0)
        TriggerLED = 1;
      else
        TriggerLED = 0;  
      digitalWrite(Output3_Pin, TriggerLED);
      FullUpdate();
      break;
    case Dkey:  //toggle state of unused dar output
      if (ExtraOut == 0)
        ExtraOut = 1;
      else
        ExtraOut = 0;  
      digitalWrite(Output4_Pin, ExtraOut);
      FullUpdate();
      break;
    case Ekey:  //Do nothing, reserved for output 5 if allowed
tone(Speaker_Pin, 262, 250);
delay(250);
noTone(Speaker_Pin);
//      if (testmode == 1)
//        digitalWrite(Output5_Pin, !(digitalRead(Output5_Pin)));
      break;
    case Fkey:  //Do nothing, reserved for output 6 if allowed
tone(Speaker_Pin, 262, 250);
delay(250);
noTone(Speaker_Pin);
//      if (testmode == 1)
//        digitalWrite(Output6_Pin, !(digitalRead(Output6_Pin)));
      break;
    case TESTMODEkey:
       //TEST MODE - get password, then let left/right buttons activate solenoids manually
       //  ABCDEF toggle outputs
       //  print in first 3 lines: "**TEST MODE** press 'test mode' to exit"
       //  print the state of inputs
      if (testmode == 0)
      {
        Serial1.write("\r\n\r\n\r\n");
        delay(80);
        if (EnterPasscode() == 1)
        {
          testmode = 1;
          RightButtonLEDState = 2;
          LeftButtonLEDState = 2;
        }
      }
      else
      {
        testmode = 0;  
        digitalWrite(Output5_Pin, 0); //leaving test mode, shut both solenoids off just in case
        digitalWrite(Output6_Pin, 0);
        RightButtonLEDState = 0;
        LeftButtonLEDState = 1;
      }
      ClearLCD();
      FullUpdate(); //(re)print the main status window      
      break;
    case BLANKkey:
      LockScreen();
      delay(100);
      FullUpdate();
      break;
    case INFOkey:
      LeftButtonLEDState = 0;
      printsequence();  //print sequence, 3 lines at a time
      LeftButtonLEDState = 1;
      break;
    case CtlONEkey:
      LeftButtonLEDState = 0;
      loadmenu(1);  //control 1-6 - prompt and load saved sequences
      LeftButtonLEDState = 1;
      break;
    case CtlTWOkey:
      LeftButtonLEDState = 0;
      loadmenu(2);  //control 1-6 - prompt and load saved sequences
      LeftButtonLEDState = 1;
      break;
    case CtlTHREEkey:
      LeftButtonLEDState = 0;
      loadmenu(3);  //control 1-6 - prompt and load saved sequences
      LeftButtonLEDState = 1;
      break;
    case CtlFOURkey:
      LeftButtonLEDState = 0;
      loadmenu(4);  //control 1-6 - prompt and load saved sequences
      LeftButtonLEDState = 1;
      break;
    case CtlFIVEkey:
      LeftButtonLEDState = 0;
      loadmenu(5);  //control 1-6 - prompt and load saved sequences
      LeftButtonLEDState = 1;
      break;
    case CtlSIXkey:
      LeftButtonLEDState = 0;
      loadmenu(6);  //control 1-6 - prompt and load saved sequences
      LeftButtonLEDState = 1;
      break;
    case SftONEkey:
      LeftButtonLEDState = 0;
      savemenu(1);  //shift 1-6 - prompt and save sequence
      LeftButtonLEDState = 1;
      break;
    case SftTWOkey:
      LeftButtonLEDState = 0;
      savemenu(2);  //shift 1-6 - prompt and save sequence
      LeftButtonLEDState = 1;
      break;
    case SftTHREEkey:
      LeftButtonLEDState = 0;
      savemenu(3);  //shift 1-6 - prompt and save sequence
      LeftButtonLEDState = 1;
      break;
    case SftFOURkey:
      LeftButtonLEDState = 0;
      savemenu(4);  //shift 1-6 - prompt and save sequence
      LeftButtonLEDState = 1;
      break;
    case SftFIVEkey:
      LeftButtonLEDState = 0;
      savemenu(5);  //shift 1-6 - prompt and save sequence
      LeftButtonLEDState = 1;
      break;
    case SftSIXkey:
      LeftButtonLEDState = 0;
      savemenu(6);  //shift 1-6 - prompt and save sequence
      LeftButtonLEDState = 1;
      break;
    case SftF1key:
      LeftButtonLEDState = 0;
      Reset(); //prompt for reset, 
      LeftButtonLEDState = 1;
      break;
    case CtlBLANKkey:
      LeftButtonLEDState = 0;
      ChangePasscode();
      LeftButtonLEDState = 1;
      FullUpdate();
      break;
    case -1:
      break;  //nothing received, do nothing
    default:
//      Serial.print(ReceivedByte, HEX);
//      Serial.println(" ");
      break;  //unknown char received, print for debug, do nothing
  }
  if (ReceivedByte != -1) //if we got something and took action, clear off any remaining bytes to prevent accidental double presses etc. 
    while (((Serial1.read()) != -1));  
 
 //Update inputs for debounce and edge detection
  Input1Last2 = Input1Last; 
  Input1Last = digitalRead(Input1_Pin);
  Input2Last2 = Input2Last; 
  Input2Last = digitalRead(Input2_Pin);
  Input3Last2 = Input3Last; 
  Input3Last = digitalRead(Input3_Pin);
  LeftButtonLast2 = LeftButtonLast;
  LeftButtonLast = digitalRead(LeftButton_Pin);
  RightButtonLast2 = RightButtonLast;
  RightButtonLast = digitalRead(RightButton_Pin);
   
  UpdateButtonLEDs(); //update button LEDs.
  delay(1);  //Small loop delay to support debounce.  This defines minimum debounce sample time.
 
//add pin change interrupt to left button so if it is realeased, turn off solenoid immediately.
}

