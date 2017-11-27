# BsidesBASIC-2017
BSides Cape Town 2017 - Flux Capacitor Badge Code
By Dale Nunns (dale@stuff.za.net)
 
This code implements a simple BASIC interpreter, async webserver, led blinking and hostAP.
The 'telnet' console (on port 88) is implemented using ANSI/VT100 drawing functions for colour and cursor movement.

### Required libraries:
* ESP8266 Board Support for Arduino - 2.4.0-rc2 https://github.com/esp8266/Arduino/releases
* ESPAsyncTCP - Required by ESPAsyncWebServer https://github.com/me-no-dev/ESPAsyncTCP
* ESPAsyncWebServer https://github.com/me-no-dev/ESPAsyncWebServer
* ESP8266RFCTelnet - RFC compliant telnet library https://github.com/blacktigersoftware/ESP8266RFCTelnet
* FastLED - WS2812 LED control library (use library in Arduino)

#### Tools
* ESP8266FS tool to upload SPIFFS filesystem https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.3.0/ESP8266FS-0.3.0.zip
* EspExceptionDecoder - converts exceptions to something that makes sense https://github.com/me-no-dev/EspExceptionDecoder

### Issues:
* Error reporting in the interpreter is badly implemented due to not having Exceptions.

### Badge 'plan' - TO DO List
#### HostAP:
* Serial no of the badge as part of the SSID
* Pressing Program Button displays SSID as 4bit hex values on badge LED's & via Serial

#### Web-interface:
* Web-interface to prompt for button press + random unlock code displayed on LED's
* Interactive SVG badge to set LED's + Program LED animation.
* Set name/handle + password for badge
* Upload/Download .BAS files ?

#### Serial:
* BASIC interpreter access after killing AUTORUN.BAS 

#### BASIC:
* CTRL+C to break out of running .BAS file in BASIC
* add INPUT and GETKEY commands to be able to handle keyboard input in BASIC
* wrap WIFI client API
* set password / disable HostAP via BASIC
* add HTTP GET commands to BASIC 

#### RF:
* RF commands need to be in BASIC + in Web Interface


## BASIC COMMANDS
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

## INTERPRETER COMMANDS
**MEM** - show free memory

**NEW** - clear current program

**CLEAR** - reset interpreter state (reset variables)

**LIST** - list current program

**LOAD** - load program from flash

**SAVE** - save program to flash

**DIR** - list files in flash

**DEL** - delete specified file from flash

**DEBUG** - enable debug mode (development)
