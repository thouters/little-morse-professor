#ifndef MORSETABLE_H
#define MORSETABLE_H
#ifdef ARDUINO
#include <avr/pgmspace.h> // Required for PROGMEM
#else
#define PROGMEM
#endif

// Define each Morse code string as a separate variable
const char space[] PROGMEM = " ";
const char exclamation[] PROGMEM = "-.-.--";
const char doubleQuote[] PROGMEM = ".-..-.";
const char hash[] PROGMEM = "#";
const char dollar[] PROGMEM = "...-..-";
const char percent[] PROGMEM = "%";
const char ampersand[] PROGMEM = ".-...";
const char singleQuote[] PROGMEM = ".----.";
const char leftParen[] PROGMEM = "-.--.";
const char rightParen[] PROGMEM = "-.--.-";
const char asterisk[] PROGMEM = "*";
const char plus[] PROGMEM = ".-.-.";
const char comma[] PROGMEM = "--..--";
const char hyphen[] PROGMEM = "-....-";
const char period[] PROGMEM = ".-.-.-";
const char slash[] PROGMEM = "-..-.";
const char zero[] PROGMEM = "-----";
const char one[] PROGMEM = ".----";
const char two[] PROGMEM = "..---";
const char three[] PROGMEM = "...--";
const char four[] PROGMEM = "....-";
const char five[] PROGMEM = ".....";
const char six[] PROGMEM = "-....";
const char seven[] PROGMEM = "--...";
const char eight[] PROGMEM = "---..";
const char nine[] PROGMEM = "----.";
const char colon[] PROGMEM = "---...";
const char semicolon[] PROGMEM = "-.-.-.";
const char lessThan[] PROGMEM = " ";
const char equals[] PROGMEM = "-...-";
const char greaterThan[] PROGMEM = " ";
const char question[] PROGMEM = "..--..";
const char at[] PROGMEM = ".--.-.";
const char a[] PROGMEM = ".-";
const char b[] PROGMEM = "-...";
const char c[] PROGMEM = "-.-.";
const char d[] PROGMEM = "-..";
const char e[] PROGMEM = ".";
const char f[] PROGMEM = "..-.";
const char g[] PROGMEM = "--.";
const char h[] PROGMEM = "....";
const char i[] PROGMEM = "..";
const char j[] PROGMEM = ".---";
const char k[] PROGMEM = "-.-";
const char l[] PROGMEM = ".-..";
const char m[] PROGMEM = "--";
const char n[] PROGMEM = "-.";
const char o[] PROGMEM = "---";
const char p[] PROGMEM = ".--.";
const char q[] PROGMEM = "--.-";
const char r[] PROGMEM = ".-.";
const char s[] PROGMEM = "...";
const char t[] PROGMEM = "-";
const char u[] PROGMEM = "..-";
const char v[] PROGMEM = "...-";
const char w[] PROGMEM = ".--";
const char x[] PROGMEM = "-..-";
const char y[] PROGMEM = "-.--";
const char z[] PROGMEM = "--..";
const char leftBracket[] PROGMEM = "[";
const char backslash[] PROGMEM = "\\";
const char rightBracket[] PROGMEM = "]";
const char caret[] PROGMEM = "^";
const char underscore[] PROGMEM = "..--.-";

// Define the Morse code table as an array of pointers to the variables
const char* const morsecode_table[] PROGMEM = {
    space,       // 32 - [SPACE]
    exclamation, // 33 - !
    doubleQuote, // 34 - "
    hash,        // 35 - #
    dollar,      // 36 - $
    percent,     // 37 - %
    ampersand,   // 38 - &
    singleQuote, // 39 - '
    leftParen,   // 40 - (
    rightParen,  // 41 - )
    asterisk,    // 42 - *
    plus,        // 43 - +
    comma,       // 44 - ,
    hyphen,      // 45 - -
    period,      // 46 - .
    slash,       // 47 - /
    zero,        // 48 - 0
    one,         // 49 - 1
    two,         // 50 - 2
    three,       // 51 - 3
    four,        // 52 - 4
    five,        // 53 - 5
    six,         // 54 - 6
    seven,       // 55 - 7
    eight,       // 56 - 8
    nine,        // 57 - 9
    colon,       // 58 - :
    semicolon,   // 59 - ;
    lessThan,    // 60 - <
    equals,      // 61 - =
    greaterThan, // 62 - >
    question,    // 63 - ?
    at,          // 64 - @
    a,           // 65 - A
    b,           // 66 - B
    c,           // 67 - C
    d,           // 68 - D
    e,           // 69 - E
    f,           // 70 - F
    g,           // 71 - G
    h,           // 72 - H
    i,           // 73 - I
    j,           // 74 - J
    k,           // 75 - K
    l,           // 76 - L
    m,           // 77 - M
    n,           // 78 - N
    o,           // 79 - O
    p,           // 80 - P
    q,           // 81 - Q
    r,           // 82 - R
    s,           // 83 - S
    t,           // 84 - T
    u,           // 85 - U
    v,           // 86 - V
    w,           // 87 - W
    x,           // 88 - X
    y,           // 89 - Y
    z,           // 90 - Z
    leftBracket, // 91 - [
    backslash,   // 92 - slash
    rightBracket,// 93 - ]
    caret,       // 94 - ^
    underscore,  // 95 - _
    nullptr      // End of table
};

#endif // MORSETABLE_H