# BsidesBASIC-2017
BSides Cape Town 2017 - Flux Capacitor Badge Code
By Dale Nunns (dale@stuff.za.net)
 
This code implements a simple BASIC interpreter, currently it only supports floats.
The serial console is implemented using ANSI/VT100 drawing functions for colour and cursor movement.

Requires:

Arduino for ESP8266 

Issues:

  Currently error reporting in the interpreter is badly implemented due to not having Exceptions.

<<<<<<< HEAD
=======
To do:
  RF - Interface to RF badge + rfcat functions.
  INPUT command - some way to receive text input.
  GETKEY command - some way to get keypresses.
  CTRL+BREAK (C) - be able to BREAK a running app.

>>>>>>> 4170454a8047ea3cd56f7f366c06b320ed84297a
BASIC COMMANDS
---
**REM** - comment line

**LET** - assign value to a variable

**IF THEN** - conditional statement

**GOTO** - jump to line number

**GOSUB / RETURN** - go to line and then return

**DO / LOOP WHILE/UNTIL** - loop until/while

**FOR..NEXT** - for next loops"

**PRINT** - print text

**PRINTLN** - print text as line

**SLEEP** - sleep for x milli-seconds

**CLS** - clear screen

**MOVE** - move the cursor to the specified x,y position on the screen.

**SET COLOUR** - set the text forecolour

**SET BACK** - set the text background colour

**END** - Specify the end of a 

**LED ON/OFF** - turn an LED on/off 

INTERPRETER COMMANDS
---
**MEM** - show free memory

**NEW** - clear current program

**CLEAR** - reset interpreter state (reset variables)

**LIST** - list current program

**LOAD** - load program from flash

**SAVE** - save program to flash

**DIR** - list files in flash

**DEL** - delete specified file from flash

**DEBUG** - enable debug mode (development)
