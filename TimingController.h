
const int Input1_Pin = 11;  //DB15 input through Opto.  High means input grounded
const int Input2_Pin = 4;  //DB15 input through Opto.  High means input grounded
const int Input3_Pin = A5;  //DB15 input through Opto.  High means input grounded
const int Output1_Pin = 5; //DB15 output thruogh darlington array.  High means drive output to ground.
const int Output2_Pin = 6; //DB15 output thruogh darlington array.  High means drive output to ground.
const int Output3_Pin = 9; //DB15 output thruogh darlington array.  High means drive output to ground.
const int Output4_Pin = 10; //DB15 output thruogh darlington array.  High means drive output to ground.
const int Output5_Pin = 12; //DB15 output thruogh darlington array.  High means drive output to ground.
const int Output6_Pin = A4; //DB15 output thruogh darlington array.  High means drive output to ground.
const int SoftSerialTX_Pin = 7; //output towards the RS232 level shifter for software serial on RJ11
const int SoftSerialRX_Pin = 8; //input from the RS232 level shifter for software serial on RJ11
const int Speaker_Pin = 13; //output through cap to speaker drive transistor.  Use PWM to drive.
const int LeftButton_Pin = 15; //left side button input.  Low means pressed
const int RightButton_Pin = 14; //right side button input.  Low means pressed
const int LeftLED_Pin = A3; //left side button LED.  low means turn led on.
const int RightLED_Pin = A2; //right side button LED.  low means turn led on.
const int SCL_Pin = 3;  
const int SDA_Pin = 2;  
const int AnalogIn_Pin = A0;

#define FlashSpeed 200 //number of loops to run before changing state of flashing led
#define LaunchDelay 50 //delay ms to wait after launch is a go before checking again.
#define maxsteps 20 //maximum number of steps to support.
  //caution not to exceed eeprom storage.
  //eeprom mapping currently places 6 saves on 128 byte boundaries
  //eeprom has last 16 bytes taken by the saved passcode
  //this means in theory maximum of 21 step
  
  
//Keypad definitions
//pressing the button will send these bytes back.
//commented out lines for shifted/control versions means the value is duplicate to the 
//   non-shifted/non-control version, or is 00 so not uniquely identifiable

//Button pressed alone:  (no shift or control)
#define CHANGEkey 0x11
#define CANXkey 0x41
#define TESTkey 0x46
#define STATUSkey 0x4b
#define SOFTFUNCkey 0x50
#define CHANGEALLkey 0x52
#define BROADCASTkey 0x54
#define ALARMkey 0x56
#define F1key 0x59
#define LEFTkey 0x12
#define TDkey 0x42
#define ONEkey 0x47
#define FOURkey 0x4c
#define SEVENkey 0x51
#define ENAkey 0x52
#define ESCAPEkey 0x55
#define INFOkey 0x57
#define F2key 0x5a
#define RIGHTkey 0x13
#define CCkey 0x43
#define TWOkey 0x48
#define FIVEkey 0x4D
#define EIGHTkey 0x37
#define ZEROkey 0x34
#define HREGkey 0x31
#define CURSORkey 0x58
#define F3key 0x20
#define DIGROUPkey 0x14
#define BLANKkey 0x44
#define THREEkey 0x49
#define SIXkey 0x4e
#define NINEkey 0x38
#define DISkey 0x35
#define TESTMODEkey 0x32
#define ENTERkey 0x30
#define F4key 0x0d
#define BROADCAST2key 0x15
#define Akey 0x45
#define Bkey 0x4a
#define Ckey 0x4f
#define Dkey 0x39
#define Ekey 0x36
#define Fkey 0x33

//Control versions:
#define CtlCHANGEkey 0x66
#define CtlCANXkey 0x01
#define CtlTESTkey 0x06
#define CtlSTATUSkey 0x0b
#define CtlSOFTFUNCkey 0x10
#define CtlCHANGEALLkey 0x12
#define CtlBROADCASTkey 0x14
#define CtlALARMkey 0x16
#define CtlF1key 0x19
#define CtlLEFTkey 0x67
#define CtlTDkey 0x02
#define CtlONEkey 0x07
#define CtlFOURkey 0x0c
//#define CtlSEVENkey 0x11
#define CtlENACtlkey 0x13
#define CtlESCAPEkey 0x15
#define CtlINFOkey 0x17
#define CtlF2key 0x1a
#define CtlRIGHTkey 0x68
#define CtlCCkey 0x03
#define CtlTWOkey 0x08
#define CtlFIVEkey 0x0d
//#define CtlEIGHTkey 0x
//#define CtlZEROkey 0x
//#define CtlHREGkey 0x
//#define CtlCURSORkey 0x
//#define CtlF3key 0x
#define CtlDIGROUPkey 0x69
#define CtlBLANKkey 0x04
#define CtlTHREEkey 0x09
#define CtlSIXkey 0x0e
//#define CtlNINEkey 0x
//#define CtlDISkey 0x
//#define CtlTESTMODEkey 0x
//#define CtlENTERkey 0x
//#define CtlF4key 0x
#define CtlBROADCAST2key 0x6a
#define CtlAkey 0x05
#define CtlBkey 0x0a
#define CtlCkey 0x0f
//#define CtlDkey 0x
//#define CtlEkey 0x
//#define CtlFkey 0x

//Shift versions:
#define SftCHANGEkey 0x61
#define SftCANXkey 0x2b
#define SftTESTkey 0x2d
#define SftSTATUSkey 0x2f
#define SftSOFTFUNCkey 0x2a
#define SftCHANGEALLkey 0x3d
#define SftBROADCASTkey 0x5e
#define SftALARMkey 0x2c
#define SftF1key 0x22
#define SftLEFTkey 0x62
#define SftTDkey 0x28
#define SftONEkey 0x21
#define SftFOURkey 0x26
#define SftSEVENkey 0x40
#define SftENAkey 0x23
#define SftESCAPEkey 0x24
#define SftINFOkey 0x25
#define SftF2key 0x1b
#define SftRIGHTkey 0x63
#define SftCCkey 0x29
#define SftTWOkey 0x5f
#define SftFIVEkey 0x3a
//#define SftEIGHTkey 0x37
//#define SftZEROkey 0x
//#define SftHREGkey 0x
#define SftCURSORkey 0x2e
#define SftF3key 0x08
#define SftDIGROUPkey 0x64
#define SftBLANKkey 0x3d
#define SftTHREEkey 0x5b
#define SftSIXkey 0x3b
//#define SftNINEkey 0x
//#define SftDISkey 0x
//#define SftTESTMODEkey 0x
//#define SftENTERkey 0x
//#define SftF4key 0x
#define SftBROADCAST2key 0x65
#define SftAkey 0x3e
#define SftBkey 0x5d
#define SftCkey 0x3f
//#define SftDkey 0x
//#define SftEkey 0x
//#define SftFkey ox

