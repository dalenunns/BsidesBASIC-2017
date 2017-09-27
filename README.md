# BsidesBASIC-2017
BSides Cape Town 2017 - Flux Capacitor Badge Code
By Dale Nunns (dale@stuff.za.net)
 
This code implements a simple BASIC interpreter, currently it only supports floats.
The serial console is implemented using ANSI/VT100 drawing functions for colour and cursor movement.

Requires:

Arduino for ESP8266 

Issues:

  Currently error reporting in the interpreter is badly implemented due to not having Exceptions.

BASIC COMMANDS
---
**REM** - comment line

**LET** - assign value to a variable

**IF THEN** - conditional statement

**GOTO** - jump to line number

**GOSUB / RETURN** - go to line and then return

**PRINT** - print text

**PRINTLN** - print text as line

**SLEEP** - sleep for x milli-seconds

**CLS** - clear screen

**MOVE** - move the cursor to the specified x,y position on the screen.

**SET COLOUR** - set the text forecolour

**SET BACK** - set the text background colour

**END** - Specify the end of a program

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
