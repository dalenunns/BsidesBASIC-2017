#include <map>
#include <stack>
#include <list>
#include <cctype>
#include <algorithm>
#include <FS.h>

//Wifi related functions
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <EEPROM.h>
/*
   BSides Cape Town - BASIC for ESP8266
   By Dale Nunns (dale@stuff.za.net)

   This code implements a simple BASIC interpreter, currently it only supports floats.
   The serial console is implemented using ANSI/VT100 drawing functions for colour and cursor movement.

   Issues:
     Currently error reporting in the interpreter is badly implemented due to not having Exceptions.
*/

#define TELNET_PORT 88 //Set to port 88 to avoid having to negotiate the telnet RFC

//FastLED library to control LED's
#define FASTLED_ESP8266_RAW_PIN_ORDER //Sets the Pin Order to RAW mode see https://github.com/FastLED/FastLED/wiki/ESP8266-notes

#include "FastLED.h"

#include "BsidesBASIC.h"

const char* http_username = "admin";
const char* http_password = "admin";
#define NUM_LEDS 15 //TODO: Change this to match flux-capacitor design
CRGB leds[NUM_LEDS];

volatile bool leds_changed = false; //used to indicate to the main loop the leds have been updated
volatile bool leds_fill = false;
volatile byte led_fill_color[3] = {0, 0, 0};
bool runOnce = false;
String line = "";
int cursor = 0;
String token;
std::map<String, float> variables_float; //Holds all the float variables in BASIC
std::map<String, String> variables_string; //Holds all the string variables in BASIC

std::map<int, String> program;
std::map<int, String>::iterator current_line;
std::stack<int> stack;
std::map<String, std::list<float>> function_args;

bool DEBUG_MODE = false;

//Wifi / WebServer Code
const char *hostName = "badge2017";

AsyncWebServer server(80);

//Setup Telnet
WiFiServer telnetServer(TELNET_PORT);
WiFiClient telnetServerClient;

/*
   Because of timing/interrupts issues with driving the WS2812 'NeoPixel' LED's we can't have WiFi + LED's.
   The badge starts in BLINKY MODE as some badges (like @HypnZA) are unstable in the WiFi enabled mode.
*/
#define BLINKY_MODE 0
#define WIFI_MODE 1

byte BADGE_MODE = BLINKY_MODE;
/*

   Main Arduino setup() method.

*/
void setup()
{
  EEPROM.begin(512); //ensure the EEPROM is initialised

  //Serial is always available.
  Serial.begin(9600); //Serial port is currently set to 9600 Baud for the Serial Console.
  Serial.setTimeout(100000); //Timeout needs to be set higher than usual otherwise you can get timeouts waiting for keys etc.

  //Setup the SPIFFS file system
  SPIFFS.begin();

  //Pre-populate array with valid functions.
  //TODO: Change this as its a waist of memory in its current form.
  function_args["rnd"] = std::list<float>();

  //Check if our "go to config" byte is set in the eeprom.
  byte eepromMode = EEPROM.read(0);

  if (eepromMode == 0x80) {
    EEPROM.write(0, 0x00);
    EEPROM.commit();

    SetupWifiConfigMode();
  } else {
    //Default is blinky mode (serial interface & LED's only)
    SetupBlinkyMode();
  }
}

//Setups Blinky mode which is running the BASIC interpreter on a serial console and the LED's
void SetupBlinkyMode() {
  BADGE_MODE = BLINKY_MODE;
  //Make sure WIFI is off.
  WiFi.mode(WIFI_OFF);
  //Setup WS2812's on pin GPIO 12 (assuming RAW pin layout)
  FastLED.addLeds<NEOPIXEL, 12>(leds, NUM_LEDS);
  //Set the default brightness low so that we don't blink people / waste battery life
  FastLED.setBrightness(32);
  FastLED.clear();
  FastLED.show();

  //Run the start up animation
  RainbowRingAnimation();

  printStartupHeader();
  //Display the '>' prompt and wait for input
  run_startup();
  print("> ");
}

void LaunchWifiConfigMode() {
  stop_program();
  StartBurstAnimation();
  if (!runOnce) {
    runOnce = true;
    EEPROM.write(0, 0x80);
    EEPROM.commit();
    delay(100);
    ESP.restart();
  }
}

//Sets up WIFI config mode which runs the HTTP server, badge emulator and telnet. NO LED's IN THIS MODE
void SetupWifiConfigMode() {
  stop_program();
  BADGE_MODE = WIFI_MODE;
  if (!runOnce) {
    runOnce = true;
    //Quick animation to show you're going into config mode, leave some LED's on so you can find victim's :-)
    String ssid = "BADGE_" + getMacAddress();

    WiFi.softAP(ssid.c_str()); //Enable the soft AP, no password specified to make life easier for now.
    delay(100); //Delay to give the WIFI a chance to turn on.

    //Setup WebServer
    IPAddress myIP = WiFi.softAPIP();
    WiFi.hostname(hostName);

    server.addHandler(new SPIFFSEditor(http_username, http_password));

    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });

    server.serveStatic("/ace.js", SPIFFS, "/ace.js");
    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->redirect("/edit");
    });

    server.onNotFound([](AsyncWebServerRequest * request) {
      request->send(404);
    });

    server.begin();

    telnetServer.begin();
    telnetServer.setNoDelay(true);

    Serial.println("Badge IoT configuration mode enabled");
    Serial.println("----------------------------------------------");
    Serial.print("SSID:");
    Serial.println(ssid);
    Serial.print("IP:");
    Serial.println(myIP);
    Serial.println("----------------------------------------------");

    printStartupHeader();
    //Display the '>' prompt and wait for input
    print("> ");
  }
}

void run_startup() {
  String file = "/startup.bas";
  if (SPIFFS.exists("/startup.bas")) {
    println("Launching STARTUP.BAS");
    program.clear(); //Clear the current in memory program

    if (SPIFFS.exists(file)) {
      File loadFile = SPIFFS.open(file, "r");
      while (loadFile.available()) {
        cursor = 0;
        line = loadFile.readStringUntil('\n');
        line.trim();
        parse_line();
      }
      loadFile.close();

      run_program();
    }
  }
}

/*

   Main Arduino loop() method.

   Main application code is in this method.
*/
String inputWord = "";
char inputChar;

bool isRunning = false;

byte state = 0;

void loop()
{
  //See if the button was pressed to put the badge into WiFi mode.
  if (digitalRead(0) == 0) {
    LaunchWifiConfigMode();
  }

  yield(); //give the background code a chance to run.

  switch (state) {
    case 0:
      updateTelnet();
      ++state;
      break;
    case 1:
      updateSerial();
      ++state;
      break;
    case 2:
      updateRunning();
      ++state;
      break;
    case 3:
      updateLEDs();
      ++state;
      break;
    default:
      state = 0;
      break;
  }

  ESP.wdtFeed(); //feed the watchdog timer so that it won't reset the esp.
}

void updateSerial() {
  if (Serial.available())
  {
    inputChar = Serial.read();
    Serial.print(inputChar);
    if (DEBUG_MODE) {
      Serial.print(inputChar, HEX);
    }
    if ((inputChar == 127) || (inputChar == 8))
    {
      if (inputWord.length() > 0)
      {
        inputWord = inputWord.substring(0, inputWord.length() - 1);
      }
    } else if ((inputChar == 27)) {
      if (isRunning) {
        println("ESCAPE pressed - Break");
        stop_program();
      }
    }
    else
    {
      inputWord += inputChar;
    }

    if (inputChar == '\r') {
      inputWord.trim();
      line = inputWord;
      println("");
      parse_command_line();
      println("");
      //Display the '>' prompt and wait for input
      print("> ");

      inputWord = ""; //reset everything
    }
  }
}

void updateTelnet() {
  if (BADGE_MODE == WIFI_MODE) {
    //check if there are any new clients
    if (telnetServer.hasClient()) {
      //find free/disconnected spot
      if (!telnetServerClient.connected()) {
        telnetServerClient.stop();
        telnetServerClient = telnetServer.available();
      } else {
        telnetServer.available().stop();
      }

      printStartupHeader();
      //Display the '>' prompt and wait for input
      print("> ");
      inputWord = ""; //reset everything
    }

    if (telnetServerClient && telnetServerClient.connected()) {
      if (telnetServerClient.available())
      {
        inputChar = telnetServerClient.read();
        telnetServerClient.print(inputChar);
        if ((inputChar == 127) || (inputChar == 8))
        {
          if (inputWord.length() > 0)
          {
            inputWord = inputWord.substring(0, inputWord.length() - 1);
          }
        }
        else if ((inputChar == 27)) {
          if (isRunning) {
            println("ESCAPE pressed - Break");
            stop_program();
          }
        }
        else
        {
          inputWord += inputChar;
        }

        if (inputChar == '\r') {
          inputWord.trim();
          line = inputWord;
          println("");
          parse_command_line();
          println("");
          //Display the '>' prompt and wait for input
          print("> ");

          inputWord = ""; //reset everything
        }
      }
    }
  }
}

void updateRunning() {
  if (isRunning) {
    if (current_line != program.end()) {
      line = current_line->second;
      cursor = 0;
      ++current_line;
      parse_block();
    } else {
      isRunning = false;
    }
  }
}

void updateLEDs() {
  if (leds_changed) {
    if (BADGE_MODE == BLINKY_MODE) {
      FastLED.show();
    }
  }
}

//Used to indicate its in Wifi mode
void StartBurstAnimation() {
  FastLED.clear();
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  FastLED.show();

  for (int i = 0; i < 5; i++) {
    leds[i].setRGB(0, 0, 255);
    leds[9 - i].setRGB(0, 0, 255);
    leds[i + 10].setRGB(0, 0, 255);
    FastLED.show();
    leds[i].setRGB(0, 0, 0);
    leds[9 - i].setRGB(0, 0, 0);
    leds[i + 10].setRGB(0, 0, 0);
    delay(90);
  }
  //  FastLED.show();
}

//Simple startup animation
void RainbowRingAnimation() {
  FastLED.clear();
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  FastLED.show();

  leds[0].setRGB(255, 0, 0);
  leds[9].setRGB(255, 0, 0);
  leds[10].setRGB(255, 0, 0);
  FastLED.show();
  delay(90);

  leds[1].setRGB(255, 255, 0);
  leds[8].setRGB(255, 255, 0);
  leds[11].setRGB(255, 255, 0);
  FastLED.show();
  delay(90);

  leds[2].setRGB(0, 255, 0);
  leds[7].setRGB(0, 255, 0);
  leds[12].setRGB(0, 255, 0);
  FastLED.show();
  delay(90);

  leds[3].setRGB(0, 0, 255);
  leds[6].setRGB(0, 0, 255);
  leds[13].setRGB(0, 0, 255);
  FastLED.show();
  delay(90);

  leds[4].setRGB(255, 0, 255);
  leds[5].setRGB(255, 0, 255);
  leds[14].setRGB(255, 0, 255);
  FastLED.show();
  delay(180);

  for (int i = 0; i < 5; i++) {
    leds[i].setRGB(0, 0, 0);
    leds[9 - i].setRGB(0, 0, 0);
    leds[i + 10].setRGB(0, 0, 0);
    FastLED.show();
    delay(90);
  }

}

//Nothin in here should block
void parse_command_line() {
  cursor = 0;

  if (match_nocase("debug")) {
    DEBUG_MODE = !DEBUG_MODE;
    if (DEBUG_MODE) {
      println("DEBUG_MODE ON");
    } else {
      println("DEBUG_MODE OFF");
    }
  }
  else if (match_nocase("mem"))
  {
    printFreeRAM();
  }
  else if (match_nocase("list"))
  {
    list_program();
  }
  else if (match_nocase("run"))
  {
    run_program();
  } else if (match_nocase("stop")) {
    stop_program();
  }
  else if (match_nocase("clear"))
  {
    variables_float.clear();
    variables_string.clear();
  }
  else if (match_nocase("new"))
  {
    program.clear();
  }
  else if (match_nocase("help"))
  {
    print_help();
  }
  else if (match_nocase("save"))
  {
    save_program();
  }
  else if (match_nocase("load"))
  {
    load_program();
  }
  else if (match_nocase("dir"))
  {
    dir();
  }
  else if (match_nocase("del"))
  {
    del();
  } else if (match_nocase("format")) {
    if (DEBUG_MODE) {
      format();
    }
  } else if (match_nocase("view")) {
    view();
  } else if (match_nocase("flash")) {
    flash_info();
  }
  else
  {
    //If the text entered isn't a Interpreter command assume its a line of BASIC code to be parsed.
    parse_line();
  }
}

void parse_statement()
{
  if (!match_keyword()) {
    error_occurred("Syntax Error - Statement Expected");
    //throw runtime_error("Syntax Error - Statement Excpected"); //TODO Change this to a custom exception
  } else {

    String stmt = token;
    if (DEBUG_MODE) {
      println("dbg - token:" + token);
    }

    if (stmt.equalsIgnoreCase("let"))
      parse_let();
    else if (stmt.equalsIgnoreCase("print"))
      parse_print();
    else if (stmt.equalsIgnoreCase("println"))
      parse_println();
    else if (stmt.equalsIgnoreCase("if"))
      parse_if();
    else if (stmt.equalsIgnoreCase("goto"))
      parse_goto();
    else if (stmt.equalsIgnoreCase("gosub"))
      parse_gosub();
    else if (stmt.equalsIgnoreCase("return"))
      parse_return();
    else if (stmt.equalsIgnoreCase("end"))
      parse_end();
    else if (stmt.equalsIgnoreCase("do"))
      parse_do();
    else if (stmt.equalsIgnoreCase("loop"))
      parse_loop();
    else if (stmt.equalsIgnoreCase("for"))
      parse_for();
    else if (stmt.equalsIgnoreCase("next"))
      parse_next();
    else if (stmt.equalsIgnoreCase("rem"))
      cursor = line.length();
    else if (stmt.equalsIgnoreCase("set"))
      parse_set();
    else if (stmt.equalsIgnoreCase("cls"))
      parse_cls();
    else if (stmt.equalsIgnoreCase("move"))
      parse_move();
    else if (stmt.equalsIgnoreCase("sleep"))
      parse_sleep();
    else if (stmt.equalsIgnoreCase("led"))
      parse_led();
    else
      error_occurred("Unknown Statement");
    //throw runtime_error("Unknown Statement");
  }
}

void printStartupHeader() {
  println("");
  println("ESCAPE to BREAK");
  println("BsidesBASIC v0.31.337 READY");
  printFreeRAM();
}

void skip_whitespace()
{
  while (cursor < line.length() && isspace(line[cursor]))
    cursor++;
}

bool match_keyword()
{
  skip_whitespace();

  if (cursor >= line.length() || !isalpha(line[cursor]))
    return false;

  int mark = cursor;
  while (cursor < line.length() && isalpha(line[cursor]))
    cursor++;

  token = line.substring(mark, cursor);
  return true;
}

bool match_number()
{
  skip_whitespace();

  if (cursor >= line.length() || !isdigit(line[cursor]))
    return false;

  int mark = cursor;
  while (cursor < line.length() && isdigit(line[cursor]))
  {
    cursor++;
  }

  if (cursor < line.length() && line[cursor] == '.')
  {
    cursor++;
    while (cursor < line.length() && isdigit(line[cursor]))
    {
      cursor++;
    }
  }

  token = line.substring(mark, cursor);
  return true;
}

bool match_float_varname()
{
  skip_whitespace();

  if (cursor >= line.length() || !isalpha(line[cursor]))
    return false;

  int mark = cursor;
  while (cursor < line.length() && isalnum(line[cursor]))
    cursor++;

  token = line.substring(mark, cursor);
  return true;
}

bool match_string_varname()
{
  skip_whitespace();

  if (cursor >= line.length() || line[cursor] != '$') //All string variables start with $
    return false;

  int mark = cursor;
  while (cursor < line.length() && (isalnum(line[cursor]) || line[cursor] == '$'))
    cursor ++;

  if (DEBUG_MODE) {
    println("mark:" + String(mark));
    println("cursor:" + String(cursor));
  }

  token = line.substring(mark, cursor);
  return true;
}

bool match_string()
{
  skip_whitespace();

  if (cursor >= line.length() || line[cursor] != '"')
    return false;

  int mark = cursor;
  cursor++; //Skip the opening double quote
  while (line[cursor] != '"')
  {
    cursor++;
    if (cursor >= line.length())
    {
      error_occurred("Unclosed string");
      return false;
      //throw runtime_error("Unclosed std::string");
    }
  }
  cursor++;                                     //skip the closing double quote
  token = line.substring(mark + 1, cursor - 1); //save the string without quotes
  return true;
}

bool match_eol()
{
  skip_whitespace();
  if (DEBUG_MODE) {
    print("eol cursor:");
    print(cursor);
    print(" len:");
    print((int)line.length());
    println("");
    println("dbg - line:" + line);
  }
  return (cursor >= line.length());
}

bool match(String text)
{
  skip_whitespace();
  if ((cursor + text.length()) <= line.length())
  {
    if (line.substring(cursor, cursor + text.length()).equals(text))
    {
      cursor += text.length();
      return true;
    }
    else
    {
      return false;
    }
  }
  return false;
}

float parse_comparison()
{
  float lside = parse_expression();

  if (!match_relation())
  {
    return lside;
  }
  else
  {
    String op = token;
    float rside = parse_expression();

    if (op == "<=")
    {
      return -(lside <= rside);
    }
    else if (op == "<")
    {
      return -(lside < rside);
    }
    else if (op == "=")
    {
      return -(lside == rside);
    }
    else if (op == "<>")
    {
      return -(lside != rside);
    }
    else if (op == ">")
    {
      return -(lside > rside);
    }
    else if (op == ">=")
    {
      return -(lside >= rside);
    }
  }
}

bool match_relation()
{
  if (match("="))
  {
    token = "=";
    return true;
  }
  else if (match("<>"))
  {
    token = "<>";
    return true;
  }
  else if (match("<="))
  {
    token = "<=";
    return true;
  }
  else if (match(">="))
  {
    token = ">=";
    return true;
  }
  else if (match("<"))
  {
    token = "<";
    return true;
  }
  else if (match(">"))
  {
    token = ">";
    return true;
  }

  return false;
}

float parse_disjunction()
{
  float lside = parse_conjunction();
  while (match_nocase("or"))
  {
    float rside = -(parse_conjunction() != 0);
    lside = -(lside != 0 || rside != 0);
  }
  return lside;
}

float parse_conjunction()
{
  float lside = parse_negation();
  while (match_nocase("and"))
  {
    float rside = -(parse_negation() != 0);
    lside = -(lside != 0 && rside != 0);
  }
  return lside;
}

float parse_negation()
{
  if (match_nocase("not"))
  {
    return -(parse_comparison() == 0);
  }
  else
  {
    return parse_comparison();
  }
}

bool match_nocase(String kw)
{
  int mark = cursor;
  if (!match_keyword())
  {
    cursor = mark;
    return false;
  }
  else if (!token.equalsIgnoreCase(kw))
  {
    cursor = mark;
    return false;
  }
  else
  {
    return true;
  }
}

float parse_expression()
{
  float t1 = parse_term();
  while (match_add_sub())
  {
    String op = token;
    float t2 = parse_term();
    if (op == "+")
    {
      t1 += t2;
    }
    else if (op == "-")
    {
      t1 -= t2;
    }
    else
    {
      error_occurred("Error:" + op);
      //throw runtime_error("Error: "+op);
    }
  }
  return t1;
}

float parse_term()
{
  float t1 = parse_factor();
  while (match_mul_div())
  {
    String op = token;
    float t2 = parse_factor();
    if (op == "*")
    {
      t1 *= t2;
    }
    else if (op == "/")
    {
      t1 /= t2;
    }
    else
    {
      error_occurred("Error:" + op);
      //throw runtime_error("Error: "+op);
    }
  }
  return t1;
}

std::list<float> parse_args() {
  if (match("(")) {
    if (match(")")) {
      return std::list<float>();
    }
    std::list<float> args (parse_expression());
    while (match(",")) {
      args.push_back(parse_expression());
    }
    if (match(")")) {
      return args;
    } else {
      error_occurred("Syntax Error Missing ')'");
    }
  } else {
    return std::list<float>();
  }
}

float parse_factor()
{
  int signnum = 1;
  if (match("-"))
  {
    signnum = -1;
  }

  if (match_number())
  {
    return token.toFloat() * signnum;
  }
  else if (match_float_varname())
  {
    token.toLowerCase();
    if (function_args.find(token) != function_args.end()) {
      std::list<float> args = parse_args();
      return call_fn(token, args) * signnum;
    }
    else if (variables_float.find(token) != variables_float.end()) {
      return variables_float[token] * signnum;
    } else {
      error_occurred("Var not found");
      //throw runtime_error("Var not found");
    }

  }
  else if (match("("))
  {
    float value = parse_expression();
    if (match(")"))
      return value * signnum;
    else
      error_occurred("Missing ')')");
    //throw runtime_error("Missing ')'");
  }
  else
  {
    error_occurred("Expression Expected");
    //throw runtime_error("Expression expected");
  }
}

float call_fn(String name, std::list<float> args) {
  if (args.size() != function_args[name].size()) {
    error_occurred("Bad argument count");
  } else if (name == "rnd") {
    return random(1000);
  } else {
    error_occurred("Unknown function");
  }
}

bool match_add_sub()
{
  if (match("+"))
  {
    token = "+";
    return true;
  }
  else if (match("-"))
  {
    token = "-";
    return true;
  }
  else
  {
    return false;
  }
}

bool match_mul_div()
{
  if (match("*"))
  {
    token = "*";
    return true;
  }
  else if (match("/"))
  {
    token = "/";
    return true;
  }
  else
  {
    return false;
  }
}

bool parse_let()
{

  if (match_string_varname()) {
    String var_name = String(token);
    var_name.toLowerCase();

    if (DEBUG_MODE) {
      println("var_name:" + var_name);
      println("cursor:" + String(cursor));
    }

    if (!match("="))
    {
      error_occurred("'=' expected");
      //throw runtime_error("'=' expected");
      return false;
    }

    if (!match_string())
    {
      error_occurred(" no string found");
      return false;
    }

    variables_string[var_name] = token;
    return true;
  }

  else if (!match_float_varname())
  {
    error_occurred("Variable expected");
    //throw runtime_error("Variable expected");
    return false;
  }

  String var_name = String(token);
  var_name.toLowerCase();

  if (!match("="))
  {
    error_occurred("'=' expected");
    //throw runtime_error("'=' expected");
    return false;
  }

  variables_float[var_name] = parse_expression();
  return true;
}

String parse_value()
{
  if (match_string())
    return token;
  else if (match_string_varname()) {
    if (variables_string.find(token) != variables_string.end())
      return variables_string[token];
  }
  else
    return String(parse_expression());
}

void parse_println()
{
  if (match_eol())
  {
    println("");
    //cout << endl; //TODO replace with generice print code
    return;
  }

  String value = parse_value();
  while (match(","))
    value += parse_value();
  println(value);

  //cout << value << endl;
}

void parse_print() {
  if (match_eol())
  {
    print("");
    //cout << endl; //TODO replace with generice print code
    return;
  }

  String value = parse_value();
  while (match(","))
    value += parse_value();
  print(value);
}

void parse_if()
{
  float condition = parse_disjunction();
  if (match_nocase("then"))
  {
    if (condition != 0)
    {
      parse_statement();
    }
    else
    {
      cursor = line.length();
    }
  }
  else
  {
    error_occurred("IF without THEN");
    //throw runtime_error("IF without THEN");
  }
}

void parse_block()
{
  parse_statement();
  while (match(":"))
  {
    parse_statement();
  }
  if (!match_eol())
    error_occurred("End of line expected");
  //throw runtime_error("End f line expected");
}

void parse_line()
{
  skip_whitespace();
  int mark = cursor;
  while (cursor < line.length() && isdigit(line[cursor]))
    cursor++;

  if (cursor > mark)
  {
    //int linenum = std::stoi(line.substr(mark, cursor-mark));
    int linenum = line.substring(mark, cursor).toInt();
    skip_whitespace();
    program[linenum] = line.substring(cursor);

    if (DEBUG_MODE) {
      println("[" + String(linenum) + "] " + line.substring(cursor));
    }
  }
  else
  {
    if (DEBUG_MODE) {
      println(">>" + line + "<<");
    }
    parse_block();
  }
}

void list_program()
{
  println("Listing Program");
  for (auto const &item : program)
  {
    print(String(item.first));
    print("\t");
    println(item.second);
  }
}

void run_program()
{
  current_line = program.begin();
  isRunning = true;
}

void stop_program() {
  println("Program stopped");
  isRunning = false;
}

void parse_goto()
{
  ESP.wdtFeed(); //feed the watchdog timer so that it won't reset the esp.

  int line_num = parse_expression();
  if (program.find(line_num) != program.end())
  {
    //cursor = (current_line->second).length(); //Makes sure it ignores everything else on a line eg: 10 GOTO 20: Print "hi" (Print "hi" won't get run)
    current_line = program.find(line_num);
    if (DEBUG_MODE) {
      println("dbg: " + current_line->second);
    }
  }
  else
  {
    error_occurred("Line not found");
    //throw runtime_error("Line not found");
  }
}

void parse_gosub() {
  int line_num = parse_expression();
  if (program.find(line_num) != program.end())
  {
    stack.push(current_line->first); //Store the current position of the application in the stack
    current_line = program.find(line_num);
  } else {
    error_occurred("Line not found");
  }
}

void parse_end() {
  current_line = program.end();
}

void parse_return() {
  if (stack.size() > 0) {
    current_line = program.find(stack.top()); //Get the last position off the stack so the program can continue from that point.
    stack.pop();
  } else {
    error_occurred("Stack underflow");
  }
}

void parse_do() {
  stack.push(current_line->first);
  cursor = line.length();
}

void parse_for() {
  if (!match_float_varname()) {
    error_occurred("Variable Expected - FOR");
    return;
  }
  token.toLowerCase();
  String var_name = token;

  if (DEBUG_MODE) {
    print("var_name:");
    println(var_name);

    print("cursor:");
    println(String(cursor));

    print("line:");
    println(line);
  }

  if (!match("=")) {
    error_occurred("'=' expected - FOR");
    return;
  }

  variables_float[var_name] = parse_expression();

  if (!match_nocase("to")) {
    error_occurred("'to' expected - FOR");
    return;
  }

  int limit = parse_expression();
  int step = 1;
  if (match_nocase("step")) {
    step = parse_expression();
    if (step == 0)
    {
      error_occurred("Infinite loop - FOR");
      return;
    }
  }

  stack.push(current_line->first); //Add the current line number to the stack.
  stack.push(limit);
  stack.push(step);
}

void parse_next() {
  ESP.wdtFeed(); //feed the watchdog timer so that it won't reset the esp.
  if (!match_float_varname()) {
    error_occurred("Variable Expected - NEXT");
    return;
  }
  token.toLowerCase();
  String var_name = token;

  if (variables_float.find(token) == variables_float.end()) {
    error_occurred("Variable not found - NEXT");
    return;
  }

  int step = stack.top();
  stack.pop();
  int limit = stack.top();
  stack.pop();
  int next_line_no = stack.top();
  stack.pop();

  variables_float[token] += step; //Add the step value to the variable.
  bool done = false;

  if (step > 0) {
    done = variables_float[token] > limit;
  } else if (step < 0) {
    done = variables_float[token] < limit;
  }

  if (!done) {
    current_line = program.find(next_line_no);
    cursor = line.length();

    stack.push(next_line_no);
    stack.push(limit);
    stack.push(step);
  }
}

void parse_loop() {
  ESP.wdtFeed(); //feed the watchdog timer so that it won't reset the esp.
  if (match_nocase("while")) {
    if (parse_disjunction()) {
      current_line = program.find(stack.top());
      cursor = line.length();
    } else {
      stack.pop();
    }
  } else if (match_nocase("until")) {
    if (parse_disjunction()) {
      stack.pop();
    } else {
      current_line = program.find(stack.top());
      cursor = line.length();
    }
  } else {
    error_occurred("Condition Expected");
  }
}

void set_foreColour(int c) {
  if ((c > 7) && (c < 16)) {
    c = c - 8;
    c = get_ansiColourCode(c); //Convert the colour to a ansi colour.
    if (BADGE_MODE == WIFI_MODE) {
      if (telnetServerClient && telnetServerClient.connected()) {
        telnetServerClient.printf("\x1b[1;%dm", c); //Bright
      }
    }

    Serial.printf("\x1b[1;%dm", c); //Bright
  }  else if (c <= 7) {
    c = get_ansiColourCode(c); //Convert the colour to a ansi colour.
    if (BADGE_MODE == WIFI_MODE) {
      if (telnetServerClient && telnetServerClient.connected()) {
        telnetServerClient.printf("\x1b[22;%dm", c); //Dim
      }
    }
    Serial.printf("\x1b[22;%dm", c); //Dim
  }
}

//Returns a ANSI colour code
int get_ansiColourCode(int c) {
  /* BASIC Colours are as follows
      0 Black
      1 Red
      2 Green
      3 Yellow
      4 Blue
      5 Magenta
      6 Cyan
      7 white

     ANSI colour code is the above + 30
  */
  return c + 30;
}

void set_backColour(int c) {
  if ((c > 7) && (c < 16)) {
    c = c - 8;
    c = get_ansiColourCode(c) + 10; //Convert the colour to a ansi colour + 10 to shift it to a background colour.
    if (BADGE_MODE == WIFI_MODE) {
      if (telnetServerClient && telnetServerClient.connected()) {
        telnetServerClient.printf("\x1b[1;%dm", c); //Bright
      }
    }
    Serial.printf("\x1b[1;%dm", c); //Bright

  }  else if (c <= 7) {
    c = get_ansiColourCode(c) + 10; //Convert the colour to a ansi colour + 10 to shift it to a background colour.
    if (BADGE_MODE == WIFI_MODE) {
      if (telnetServerClient && telnetServerClient.connected()) {
        telnetServerClient.printf("\x1b[22;%dm", c); //Dim
      }
    }
    Serial.printf("\x1b[22;%dm", c); //Dim

  }
}

void parse_set() {
  if (match_nocase("colour")) { //text fore colour
    int foreColour = parse_expression();
    set_foreColour(foreColour);
  } else if (match_nocase("back")) { //text back color
    int backColour = parse_expression();
    set_backColour(backColour);
  } else if (match_nocase("term")) { //terminal type

  }
}

void parse_move() {
  int x = parse_expression();
  skip_whitespace();
  if (!match(",")) {
    error_occurred("Missing ,");
    return;
  }
  int y = parse_expression();
  if (BADGE_MODE == WIFI_MODE) {

    if (telnetServerClient && telnetServerClient.connected()) {
      telnetServerClient.printf("\x1b[%d;%dH", y, x);
    }
  }
  Serial.printf("\x1b[%d;%dH", y, x);

}

void parse_sleep() {
  int s = parse_expression();
  for (int d = 0; d < s; d++) {
    ESP.wdtFeed(); //feed the watchdog timer so that it won't reset the esp.
    delay(1);
  }
}

void parse_cls() {
  if (BADGE_MODE == WIFI_MODE) {
    if (telnetServerClient && telnetServerClient.connected()) {
      telnetServerClient.printf("\x1b[2J \x1b[0;0H");
    }
  }
  Serial.printf("\x1b[2J \x1b[0;0H");

}


void parse_led_fill() {
  byte r = 0;
  byte g = 0;
  byte b = 0;

  skip_whitespace();
  if (match_nocase("rgb")) {
    skip_whitespace();
    if (!match("(")) {
      error_occurred("Missing (");
      return;
    }
    skip_whitespace();
    r = parse_expression();

    if (!match(",")) {
      error_occurred("Missing ,");
      return;
    }
    skip_whitespace();
    g = parse_expression();

    if (!match(",")) {
      error_occurred("Missing ,");
      return;
    }
    skip_whitespace();
    b = parse_expression();

    if (!match(")")) {
      error_occurred("Missing )");
      return;
    }
  }

  if (BADGE_MODE == BLINKY_MODE) {
    fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
    leds_changed = true;
  }
}


void parse_led_brightness() {
  int b = parse_expression();

  if ((b > 0) && (b <= 255)) {
    if (BADGE_MODE == BLINKY_MODE) {
      FastLED.setBrightness(b);
      leds_changed = true;
    }
  }
}

void parse_led_set() {
  byte r = 0;
  byte g = 0;
  byte b = 0;
  skip_whitespace();

  byte ledNo = parse_expression();

  skip_whitespace();

  if (match(",")) {
    skip_whitespace();
    if (match_nocase("rgb")) {
      skip_whitespace();
      if (!match("(")) {
        error_occurred("Missing (");
        return;
      }
      skip_whitespace();
      r = parse_expression();

      if (!match(",")) {
        error_occurred("Missing ,");
        return;
      }
      skip_whitespace();
      g = parse_expression();

      if (!match(",")) {
        error_occurred("Missing ,");
        return;
      }
      skip_whitespace();
      b = parse_expression();

      if (!match(")")) {
        error_occurred("Missing )");
        return;
      }
    }
  }

  if (BADGE_MODE == BLINKY_MODE) {
    leds[ledNo].setRGB(r, g, b);
  }
}

void parse_led_show() {
  if (BADGE_MODE == BLINKY_MODE) {
    leds_changed = true;
  }
}

void parse_led() {
  bool on = false;

  byte r = 0;
  byte g = 0;
  byte b = 0;

  skip_whitespace();

  if (match_nocase("fill")) {
    parse_led_fill();
  } else if (match_nocase("brightness")) {
    parse_led_brightness();
  } else if (match_nocase("set")) {
    parse_led_set();
  } else if (match_nocase("show")) {
    parse_led_show();
  } else {
    if (match_nocase("on")) {
      on = true;
    } else if (match_nocase("off")) {
      on = false;
    }

    if (!match(",")) {
      error_occurred("Missing ,");
      return;
    }

    byte ledNo = parse_expression();

    skip_whitespace();
    if (match(",")) {
      skip_whitespace();
      if (match_nocase("rgb")) {
        skip_whitespace();
        if (!match("(")) {
          error_occurred("Missing (");
          return;
        }
        skip_whitespace();
        r = parse_expression();

        if (!match(",")) {
          error_occurred("Missing ,");
          return;
        }
        skip_whitespace();
        g = parse_expression();

        if (!match(",")) {
          error_occurred("Missing ,");
          return;
        }
        skip_whitespace();
        b = parse_expression();

        if (!match(")")) {
          error_occurred("Missing )");
          return;
        }
      }
    }

    if (BADGE_MODE == BLINKY_MODE) {
      if (on)
        leds[ledNo].setRGB(r, g, b);
      else
        leds[ledNo] = CRGB::Black;

      leds_changed = true;
    }
  }
}

void print (char c) {
  if (BADGE_MODE == WIFI_MODE) {
    if (telnetServerClient && telnetServerClient.connected()) {
      telnetServerClient.print(c);
    }
  }

  Serial.print(c);

}

void print(String s)
{
  if (BADGE_MODE == WIFI_MODE) {
    if (telnetServerClient && telnetServerClient.connected()) {
      telnetServerClient.print(s);
    }
  }

  Serial.print(s);
}

void print(int i)
{
  if (BADGE_MODE == WIFI_MODE) {

    if (telnetServerClient && telnetServerClient.connected()) {
      telnetServerClient.print(i);
    }
  }
  Serial.print(i);
}

void println(String s)
{
  if (BADGE_MODE == WIFI_MODE) {

    if (telnetServerClient && telnetServerClient.connected()) {
      telnetServerClient.println(s);
    }
  }
  Serial.println(s);

}

void error_occurred(String msg)
{
  println(msg);
}

int freeRam()
{
  return ESP.getFreeHeap();
}

void printAsHex(String s) {
  for (int i = 0; i < s.length(); i++) {
    Serial.print((byte)s[i], HEX);
    print(" ");
  }
  println("");
}

void printFreeRAM() {
  print(freeRam());
  println(" bytes free");
}

void print_help() {
  println("BASIC COMMANDS");
  println("  REM - comment line");
  println("  LET - assign value to a variable");
  println("  IF THEN - conditional statement");
  println("  GOTO - jump to line number");
  println("  GOSUB / RETURN - go to line and then return");
  println("  DO / LOOP WHILE/UNTIL - loop until/while");
  println("  FOR..NEXT - for next loops");
  println("  PRINT - print text");
  println("  PRINTLN - print text as line");
  println("  SLEEP - sleep for x milli-seconds");
  println("  CLS - clear screen");
  println("  MOVE - move the cursor to the specified x,y position on the screen.");
  println("  SET COLOUR - set the text forecolour");
  println("  SET BACK - set the text background colour");
  println("  END - specify the end of a program");
  println("  LED ON/OFF - turn an LED on/off");
  println("  LED BRIGHTNESS - set the LED brightness (1-255)");
  println("  LED FILL - set all the LEDs to rgb(r,g,b)");
  println("  LED SET - set a specific LED to rgb(r,g,b) value");
  println("  LED SHOW - update leds to previously set values");

  println("");
  println("INTERPRETER COMMANDS");
  println("  MEM - show free memory");
  println("  RUN - run currently in memory program");
  println("  STOP - stop currently running program");
  println("  NEW - clear current program");
  println("  CLEAR - reset interpreter state (reset variables)");
  println("  LIST - list current in memory program");
  println("  LOAD - load program from flash");
  println("  SAVE - save program to flash");
  println("  DIR - list files in flash");
  println("  DEL - delete specified file from flash");
  println("  VIEW - display contents of file in flash");
  println("");
}

void load_program() {
  stop_program();
  program.clear(); //Clear the current in memory program
  String file = parse_value();
  if (!file.startsWith("/")) file = "/" + file;
  if (SPIFFS.exists(file)) {
    File loadFile = SPIFFS.open(file, "r");
    while (loadFile.available()) {
      cursor = 0;
      line = loadFile.readStringUntil('\n');
      line.trim();
      parse_line();
    }
    println("LOADED");
    loadFile.close();
  } else {
    println(file + " - FILE NOT FOUND");
  }
}

void view() {
  String file = parse_value();
  if (SPIFFS.exists(file)) {
    File loadFile = SPIFFS.open(file, "r");
    while (loadFile.available()) {
      println(loadFile.readStringUntil('\n'));
    }
    loadFile.close();
  } else {
    println(file + " - FILE NOT FOUND");
  }

}

void save_program() {
  String file = parse_value();
  if (!file.startsWith("/")) file = "/" + file;
  File saveFile = SPIFFS.open(file, "w");
  for (auto const &item : program)
  {
    ESP.wdtFeed(); //feed the watchdog timer so that it won't reset the esp.
    saveFile.print(String(item.first));
    saveFile.print(" ");
    saveFile.println(item.second);
  }
  println("SAVED");
  saveFile.close();
}

void dir() {
  Dir dir = SPIFFS.openDir("");
  while (dir.next()) {
    if ((!dir.fileName().endsWith("gz")) && (!dir.fileName().endsWith("htm")) && (!dir.fileName().startsWith("/.")) && (!dir.fileName().endsWith("ico"))) {
      println(dir.fileName());
    }
  }
}

void del() {
  String file = parse_value();
  if (!file.startsWith("/")) file = "/" + file;
  if (SPIFFS.exists(file)) {
    if (SPIFFS.remove(file)) {
      println(file + " - DELETED");
    }
  } else {
    println(file + " - FILE NOT FOUND");
  }
}

void format() {
  if (SPIFFS.format()) {
    println("Format Complete");
  } else {
    println("Format FAILED!");
  }
}

void flash_info() {
  println("Flash Chip Info:");
  println(" ID:" + String(ESP.getFlashChipId()));
  println(" Size:" + String(ESP.getFlashChipSize()));
  println(" Real Size:" + String(ESP.getFlashChipRealSize()));
  println(" Speed:" + String(ESP.getFlashChipSpeed()));
  println(" Size by Chip ID:" + String(ESP.getFlashChipSizeByChipId()));
  println(" Mode:" + String(ESP.getFlashChipMode()));
}

void onRequest(AsyncWebServerRequest * request) {
  //Handle Unknown Request
  request->send(404);
}

String getMacAddress() {
  byte mac[6];

  WiFi.macAddress(mac);
  String cMac = "";
  for (int i = 0; i < 6; ++i) {
    if (mac[i] < 0x10) {
      cMac += "0";
    }
    cMac += String(mac[i], HEX);
    if (i < 5)
      cMac += ""; // put : or - if you want byte delimiters
  }
  cMac.toUpperCase();
  return cMac;
}

