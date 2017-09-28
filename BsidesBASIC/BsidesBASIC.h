void skip_whitespace();

bool match_nocase(String kw);
bool match(String text);
bool match_string();
bool match_eol();
bool match_number();
bool match_varname();
bool match_relation();
bool match_keyword();
bool match_add_sub();
bool match_mul_div();

float parse_term();
float parse_factor();
float parse_conjunction();
float parse_disjunction();
float parse_negation();
float parse_expression();
float parse_comparison();
String parse_value();
void parse_block();
void parse_line();

void parse_statement(); //Does the actual BASIC command parsing

//BASIC commands
bool parse_let();
void parse_println();
void parse_print();
void parse_if();
void parse_goto();
void parse_gosub();
void parse_return();
void parse_end();
void parse_do();
void parse_for();
void parse_next();
void parse_loop();
void parse_set();
void parse_move();
void parse_sleep();
void parse_cls();

//Interpreter Commands
void list_program();
void run_program();
void printFreeRAM();
void print_help();
void load_program();
void save_program();
void dir();
void del();

//Terminal related helper methods
int get_ansiColourCode(int c);
void set_foreColour(int c);
void set_backColour(int c);
void print(String s);
void print(int i);
void println(String s);
String gettermline(); //Gets a line of text for the interpreter
void printAsHex(String s);

void error_occurred(String msg);

std::list<float> parse_args();

int freeRam();
void printStartupHeader();

float call_fn(String name, std::list<float> args);
