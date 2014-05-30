//Various functions for supporting displaying on the LCD


//----------------------  Basic
//|Sequence: 1 Edited  |
//|Steps: 00 Time: 0000|
//|      *ARMED*       |
//|HSIa: 1 b: 0  Key: 0|
//----------------------

//----------------------  Detailed
//|Sequence:*1 Stps: 00|
//|Disarmed Time: 00000|
//|Key: 0 Butn: 0 In3:0|
//|HSIs:10 LED:0 Out4:0|
//----------------------

//----------------------  Test Mode
//|  ** TEST MODE **   |
//|Key: 0 Butn: 0 In3:0|
//|HSIs:10 LED:0 Out4:0|
//|Sol1(Ign): 0 Sol2: 0|
//----------------------

//untested.  Can do testing to see if there is a code to clear screen instead of overprinting it.
void ClearLCD()
{
  Serial1.write("\r\n");
  Serial1.write("\r\n");
  Serial1.write("\r\n");
  Serial1.write("\r\n");
}


//untested.
void FullUpdate()
{
  if (testmode == 1)
  {
    //----------------------  Test Mode
    //|  ** TEST MODE **   |
    //|Key: 0 Butn: 0 In3:0|
    //|HSIs:10 LED:0 Out4:0|
    //|Sol1(Ign): 0 Sol2: 0|
    //----------------------
    Serial1.write("\n  ** TEST M");
    delay(80);
    Serial1.write("ODE **\r\n");
  //line 2:
    Serial1.write("Key: ");
    Serial1.print(Key);
    Serial1.write(" Butn: ");
    Serial1.print(Button);
    Serial1.write(" In3:");
    Serial1.print(ExtraIn);
    Serial1.write("\r\n");

  //line 3:
    Serial1.write("HSIs:");
    Serial1.print(HSI1);
    Serial1.print(HSI2);
    Serial1.write(" LED:");
    Serial1.print(TriggerLED);
    Serial1.write(" Out4:");
    Serial1.print(ExtraOut);
    Serial1.write("\r\n");
    
  //line 4:
    Serial1.write("Sol1(Ign): ");
    Serial1.print(digitalRead(Output5_Pin));
    Serial1.write(" Sol2: ");
    Serial1.print(digitalRead(Output6_Pin));
    Serial1.write("\r");
    return;
  }
  if (DisplayType == 0)
  {
    //----------------------  Basic
    //|Sequence: 1 Edited  |
    //|Steps: 00 Time: 0000|
    //|      *ARMED*       |
    //|HSIa: 1 b: 0  Key: 0|
    //----------------------
  //line 1:
    Serial1.write("Sequence: ");
    Serial1.print(sequencenum+1);
    delay(50);
    if (edited != 0)
      Serial1.write(" Edited  \r\n");
    else
      Serial1.print("         \r\n"); //overprint old chars; may not be needed

  //line2:
    Serial1.write("Steps: ");
    delay(10);
    Serial1.print(numsteps);
    Serial1.write(" Time:");
    unsigned long totaltime = sequencetime();
    if (totaltime <= 9999)
      Serial1.write(' ');
    if (totaltime > 99999)
    {
      totaltime = totaltime / 1000; //change to seconds
      Serial1.print(totaltime);
      Serial1.write('s');
    }
    else
      Serial1.print(totaltime);
    
    Serial1.print("\r\n"); //attempt to deal with overprinting old characters
  
  //line 3:
    delay(30);
    if (Armed == 1)
      Serial1.write("      *ARMED*   \r\n");
    else
      Serial1.write("      Disarmed  \r\n");
    
  //line 4:
    Serial1.write("HSIa: ");
    Serial1.print(HSI1);
    Serial1.write(" b: ");
    Serial1.print(HSI2);
    Serial1.write("  Key: ");
    Serial1.print(Key);
    Serial1.write("\r");
  }
  else
  {
    //----------------------  Detailed
    //|Sequence:*1 Stps: 00|
    //|Disarmed Time: 00000|
    //|Key: 0 Butn: 0 In3:0|
    //|HSIs:10 LED:0 Out4:0|
    //----------------------
  //line 1:
    Serial1.write("Sequence:");
    delay(50);
    if (edited != 0)
      Serial1.write("*");
    else
      Serial1.print(" ");
    Serial1.print(sequencenum+1);
    Serial1.write(" Stps: ");
    Serial1.print(numsteps);
    Serial1.write("\r\n");
    delay(20);
  
  //line2:
    if (Armed == 1)
      Serial1.write("*ARMED*  ");
    else
      Serial1.write("Disarmed ");
    Serial1.write("Time:");
    unsigned long totaltime = sequencetime();
    if (totaltime <= 99999)
      Serial1.write(' ');
    if (totaltime > 999999)
    {
      totaltime = totaltime / 1000; //change to seconds
      Serial1.print(totaltime);
      Serial1.write('s');
    }
    else
      Serial1.print(totaltime);

    Serial1.write("\r\n"); //attempt to deal with overprinting old characters
    delay(20);
  
  //line 3:
    Serial1.write("Key: ");
    Serial1.print(Key);
    Serial1.write(" Butn: ");
    Serial1.print(Button);
    Serial1.write(" In3:");
    Serial1.print(ExtraIn);
    Serial1.write("\r\n");
    delay(20);

    
  //line 4:
    Serial1.write("HSIs:");
    Serial1.print(HSI1);
    Serial1.print(HSI1);
    Serial1.write(" LED:");
    Serial1.print(TriggerLED);
    Serial1.write(" Out4:");
    Serial1.print(ExtraOut);
    Serial1.write("\r");
  }
    

}


//12345678901234567890
//#01 0000ms Ch1 OFF
//  END OF SEQUENCE
//ENT-cont  ESC-exit
void printsequence()
{
  byte linecounter = 0;
  for (byte i = 0; i < numsteps; i++)
  {
    Serial1.write("\r\n");
    delay(80);
    if (sequence[i][1] != 0xff)  //check that not end of sequence
    {
//implement manual counter 0->2, and pause when get to 3 (and reset)
      Serial1.write('#');
      Serial1.print(i+1);
      Serial1.write(' ');
      Serial1.print(sequence[i][0]);
      Serial1.write("ms Ch");
      Serial1.print(sequence[i][1]+1);  //print 1-6 for 0-5
      if (sequence[i][2] == 0)
        Serial1.write(" OFF");
      else
        Serial1.write(" ON");
    }
    else  //end of sequence
    {
      Serial1.write("  END OF SEQUENCE\r\n");
      break;
    }
    if (linecounter >= 2)  //check if printed 3 lines already
    {
      linecounter = 0;
      Serial1.write("\r\nESC-exit Other-cont");
      while (Serial1.available() == 0)
        delay(1);   //wait here until get a key
      if (Serial1.read() == ESCAPEkey)
      {
        ClearLCD();
        FullUpdate();
        return;
      }
      Serial1.write("\r\n");
      delay(80);
    }    
    else
      linecounter++;
  }
  Serial1.write("Press a key to exit");
  while (Serial1.available() == 0)
    delay(1);   //wait here until get a key
  ClearLCD();
  FullUpdate();
}




