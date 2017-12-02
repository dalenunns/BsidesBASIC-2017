# BsidesBASIC-2017
BSides Cape Town 2017 - Flux Capacitor Badge Code  
By Dale Nunns (dale@stuff.za.net) 
 
This code implements a simple BASIC interpreter, async webserver, led blinking and hostAP.  
Serial console and 'telnet' console (on port 88) is implemented using ANSI/VT100 drawing functions for colour and cursor movement.

### Required libraries:
* Arduino 1.8.5 (or higher) environment
* ESP8266 Board Support for Arduino - 2.4.0-rc2 https://github.com/esp8266/Arduino/releases
* ESPAsyncTCP - Required by ESPAsyncWebServer https://github.com/me-no-dev/ESPAsyncTCP
* ESPAsyncWebServer https://github.com/dalenunns/ESPAsyncWebServer (modified version of this code https://github.com/me-no-dev/ESPAsyncWebServer)
* FastLED - WS2812 LED control library (use library in Arduino)

#### Additional tools that may be helpful
* ESP8266FS - tool to upload SPIFFS filesystem https://github.com/esp8266/arduino-esp8266fs-plugin
* EspExceptionDecoder - converts exceptions to something that makes sense https://github.com/me-no-dev/EspExceptionDecoder

### Known Issues:
* Error reporting in the interpreter is badly implemented due to not having Exceptions. (Code was originally written for standard C++ and then ported to Arduino)
* Backspace does not work in all terminal emulators (works best in Putty)
* LED's are disabled in WiFi config mode (softAP mode) as they can't run while SoftAP + WebServer are running
* Listing long files in BASIC can cause a WDTReset Exception

### Build Instructions:
* Install the ESP8266 Board Support package (2.4.0-rc2 link above), follow the instructions above.
* Download the ESPAsyncTCP & ESPAsyncWebServer into the ~/Arduino/libraries/ folder.
* Install the FastLED library via the built in Arduino library manager.
* Install the ESP8266FS & EspExceptionDecoder into the ~/Arduino/tools/ folder.

### Deployment Instructions - with Arduino IDE:
* Use a power bank or plug the power cable into a computer / usb power supply. (If you're using a battery bank make sure it stays on.)
* Put the badge into programming mode.
  * If you're lucky enough to have a badge with two buttons, hold down the PROGRAM button and press RESET
  * If you have one button, hold down the PROGRAM button and unplug/plug power back in.
* Deploy the Arduino code normally via the Arduino IDE.
* Once complete, put the device back into programming mode.
* Deploy the SPIFFS file system by going Tools -> ESP8266 Sketch Data Upload
* Once complete, plug/unplug power or press reset and the badge should power up with the firmware.

### Deployment of Official Binary images:
* /bin/ contains the two binary images.
* Example commands to deploy via esptool are in the `upload-commands.txt` file

### Using the badge
#### Serial Access
* You'll need to find and install the CH430 driver for your OS.
* The badge needs to be powered via the soldered USB cable.
* Plug a standard USB cable into the other socket.
* Badge will show up a standard Serial Port. 
* Connect to it via a serial terminal emulator (Putty is what works best)
* Press ESC to stop the current running program.
* You can now write code in BASIC for the badge.

#### Wifi Mode
* *NOTE LEDS ARE DISABLED IN THIS MODE*
* With the badge powered up.
* Press the PROGRAM button on the back of the badge, the badge will display a blue pattern and then stop.
* The badge will show up with a SSID of BADGE_* (If you press PROGRAM with the serial console open it'll show the SSID on that)
* Connect to the badge SSID (its an open access point)
* The badge is accessible at `http://192.168.4.1`
* Username & password is admin \ admin (its an IoT device after all)
* You can now edit the .bas files on the device via a webpage.
* You can telnet into the badge on port 88


### SPIFFS File System ###
*/BsidesBASIC/data/* contains files for the SPIFFS file system.  
All the .bas files were written by @HypnZA [@HypnZA](https://www.twitter.com/HypnZA)  
Rest of the files are required for the web-interface and come from the ESPWebAsync project.

### BASIC Commands
*REM* - comment line  
*LET* - assign value to a variable  
*IF THEN* - conditional statement  
*GOTO* - jump to line number  
*GOSUB / RETURN* - go to line and then return  
*DO / LOOP WHILE/UNTIL* - loop until/while  
*FOR..NEXT* - for next loops  
*PRINT* - print text  
*PRINTLN* - print text as line  
*SLEEP* - sleep for x milli-seconds  
*CLS* - clear screen  
*MOVE* - move the cursor to the specified x,y position on the screen.  
*SET COLOUR* - set the text forecolour  
*SET BACK* - set the text background colour  
*END* - specify the end of a program  
*LED ON/OFF* - turn an LED on/off  
*LED BRIGHTNESS* - set the LED brightness (1-255)  
*LED FILL* - set all the LEDs to rgb(r,g,b)  
*LED SET* - set a specific LED to rgb(r,g,b) value  
*LED SHOW* - update leds to previously set values  

### Interpreter Commands
*MEM* - show free memory  
*RUN* - run currently in memory program  
*STOP* - stop currently running program  
*NEW* - clear current program  
*CLEAR* - reset interpreter state (reset variables)  
*LIST* - list current in memory program  
*LOAD* - load program from flash  
*SAVE* - save program to flash  
*DIR* - list files in flash  
*DEL* - delete specified file from flash  
*VIEW* - display contents of file in flash  
