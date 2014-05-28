/*
Instructions for operation:

Quick Key:
STATUS: Toggle/Refresh display
Unlabeled key: Enter/Exit lockout
INFO: Display the current sequence
Control 1-6: Load sequence 1-6
Shift 1-6: Save sequence 1-6
Shift-F1: Reset controller
A: Toggle state of HSIa
B: Toggle state of HSIb
C: Toggle state of main trigger button LED
CHANGE: Edit the sequence
TESTMODE: Enter/Exit testing mode
Control Unlabeled key: Change passcode

The hand controller has a 4 line by 20 character display at the top displaying system status or prompts for input.

The lower portion has a keypad for user input.  Not all keys are used.
Keys should be pressed deliberately, not as a quick press which may be missed.
For control or shift functions, press and hold the control _OR_ shift key, and press the corresponding key.

There is also a pushbutton on both the left and right sides of the controller, just below the display.  These have integrated LEDs.

The left button LED is illuminated when the system is at the main screen, and ready for operation.

The right button LED is illuminated when the system is armed, and ready to launch the sequence.

Both button LEDs will flash when the system is locked out.

The left button serves as a safety arming control.  It must be held in to arm the system which prepares for launching the sequence.
The sequence can only be launched when it is at the main screen.(see the state of left button LED to show it is ready)

The right button serves as the launch button to start the sequence.  This will only be active if the system is armed.

The main window has two display modes, the default which is easier to read, and a more detailed version which has more information.
These modes can be toggled by pressing the key labeled STATUS
The STATUS button can also be used to refresh the display.

From the main window, a number of keys serve to enter different functions.

The unlabeled key will enter lockout mode.  
Pressing it when in lockout mode will prompt the user for the password to unlock.  Press ENTER when done entering code.

Pressing A will toggle the state of HSIa, the primary hot surface ignitor.

Pressing B will toggle the state of HSIb, the secondary hot surface ignitor.

Pressing C will toggle the state of the LED on the main firing button.

Pressing D will toggle the state of the unused extra output

Pressing INFO will print out the current sequence.

Pressing CHANGE goes into a menu for editing the current sequence in memory.
Channel 1 is HSIa
Channel 2 is HSIb
Channel 3 is main firing button LED
Channel 4 is the extra output
Channel 5 is the main solenoid with ignitor
Channel 6 is the main solenoid with NO ignitor

Pressing TESTMODE will prompt for the password, and enter the testing mode, which displays all inputs and outputs, and allows toggling of outputs.
Pressing TESTMODE again will leave the testing mode.

Pressing CONTROL with the unlabeled(lockout) key will prompt for the password and allow changing of the password.

Pressing Shift with F1 will reset the controller.







*/
