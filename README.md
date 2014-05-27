SequencingController
====================

Sequencing/timing controller with safety features.

This project supports a user interface using an LCD and keypad and several buttons to configure and run a programmed firing or launch sequence with multiple channels and timing delays.  It supports 6 external output channels, and 3 external input channels, but is expandable.

Safety features include positive manual enable(an arm button must be held to allow activation), disabling outputs automatically in multiple failure cases, and passcode based lockout.  It is intending to support the requirements of NFPA-160 "Standard for Flame Effects Before an Audience" Group III flame effect, but includes support for other classifications.

Menu based alterations of various settings is included, as well as eeprom saved sequences and passcodes.

It runs on an Arduino Micro, embedded on a coherent communications hand paddle, though the paddle communication is solely over the Serial1 port of the Micro so other serial LCD/keypad based solutions should work as well.
