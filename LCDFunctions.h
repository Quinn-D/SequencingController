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
//  Serial1.write("                    ");
  Serial1.write("\r\n");
//  Serial1.write("                    ");
  Serial1.write("\r\n");
//  Serial1.write("                    ");
  Serial1.write("\r\n");
//  Serial1.write("                    ");
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
    if (Key == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write(" Butn: ");
    if (Button == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write(" In3:");
    if (ExtraIn == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write("\r\n");

  //line 3:
    Serial1.write("HSIs:");
    if (HSI1 == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    if (HSI2 == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write(" LED:");
    if (TriggerLED == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write(" Out4:");
    if (ExtraOut == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write("\r\n");
    
  //line 4:
    Serial1.write("Sol1(Ign): ");
    if (digitalRead(Output5_Pin) == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write(" Sol2: ");
    if (digitalRead(Output6_Pin) == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
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
    unsigned long totaltime = 0;
    for (byte i = 0; i < maxsteps; i++)  //quick calc the sequence time
    {
      totaltime += sequence[i][0];
      if (sequence[i][1] == 0xff)
        break;
    }
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
    if (HSI1 == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write(" b: ");
    if (HSI2 == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write("  Key: ");
    if (Key == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
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
    unsigned long totaltime = 0;
    for (byte i = 0; i < maxsteps; i++)  //quick calc the sequence time
    {
      totaltime += sequence[i][0];
      if (sequence[i][1] == 0xff)
        break;
    }

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
    if (Key == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write(" Butn: ");
    if (Button == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write(" In3:");
    if (ExtraIn == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write("\r\n");
    delay(20);

    
  //line 4:
    Serial1.write("HSIs:");
    if (HSI1 == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    if (HSI2 == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write(" LED:");
    if (TriggerLED == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
    Serial1.write(" Out4:");
    if (ExtraOut == 1)
      Serial1.write('1');
    else
      Serial1.write('0');
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
      Serial1.print(sequence[i][1]+1);
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
      Serial1.write("\r\nENT-cont  ESC-exit");
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




//---------------testing junk:

//void junksetup()
//{
//  sequence[0][0] = 100;
//  sequence[0][1] = 1;
//  sequence[0][2] = 1;
//  sequence[1][0] = 100;
//  sequence[1][1] = 1;
//  sequence[1][2] = 0;
//  sequence[2][0] = 100;
//  sequence[2][1] = 2;
//  sequence[2][2] = 1;
//  sequence[3][0] = 100;
//  sequence[3][1] = 2;
//  sequence[3][2] = 0;
//  sequence[4][0] = 0;
//  sequence[4][1] = 0xff;
//  sequence[4][2] = 0;
//}


//----------------------
//|Pre: 0000ms - 0000ms|
//|Ign: 0000ms - 0000ms|
//|      *ARMED*       |
//|HSIa: 1 b: 0  Key: 0|
//----------------------

