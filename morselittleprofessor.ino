#include "Arduino.h"

#include "professor.cpp"


#ifdef __AVR_ATmega88__
#define SW3 PIN_PC0
#define ROW_GREEN_1 PIN_PC1 
#define ROW_GREEN_2 PIN_PC2 
#define ROW_GREEN_3 PIN_PC3 
#define ROW_GREEN_4 PIN_PC4 
#define ROW_GREEN_5 PIN_PC5 
#define COLUMN1 PIN_PD0
#define COLUMN2 PIN_PD1
#define COLUMN3 PIN_PD2
#define COLUMN4 PIN_PD3
#define COLUMN5 PIN_PD4 
#define COLUMN6 PIN_PD5
#define COLUMN7 PIN_PD6
#define SW4 PIN_PD7
#define SW1 PIN_PB0 
#define SPKR PIN_PB1 
#define SW2 PIN_PB2
#define ROW_RED_1 PIN_PB3
#define ROW_RED_2 PIN_PB4 
#define ROW_RED_3 PIN_PB5 
#define ROW_RED_4 PIN_PB6
#define ROW_RED_5 PIN_PB7
#else
// arduino uno using serial port as Display
#define SW1 2 
#define SW2 3
#define SW4 4
#define SW3 5
#define SPKR 6 
#endif



#ifdef __AVR_ATmega88__
/* Render the application state on a dot matrix display.
* The display has 7 columns and 5 rows.
* Note that the schematic shows the display as 5 columns and 7 rows(mindfuck),
* we will have vertical scanlines.
* we will map all letters of the alphabet to one pixel, to be displayed horizontally,
* from left to right, wrapping to the next line.
* The bottom line is reserved to show a morse code pattern on the left
* and pixels representing each state on the right side
*/
class LedMatrixDisplay : public StateVisualizer {
private:
    State currentState; // Stores the current state (enum)
    bool letters[26];   // Array to store the enabled state of each letter (A-Z)
    bool morsePixelState; // Stores the current state of the Morse pixel (on or off)
    uint8_t currentColumn;
    char *morsePattern; // Stores the current Morse pattern

public:
    // Constructor
    LedMatrixDisplay() : currentState(SHOW), morsePixelState(false), currentColumn(0), morsePattern(nullptr) {
        // Initialize all letters to false (disabled)
        for (int i = 0; i < 26; i++) {
            letters[i] = false;
        }
        currentColumn = 0; // Initialize the current column

        pinMode(COLUMN1, OUTPUT);
        pinMode(COLUMN2, OUTPUT);
        pinMode(COLUMN3, OUTPUT);
        pinMode(COLUMN4, OUTPUT);
        pinMode(COLUMN5, OUTPUT);
        pinMode(COLUMN6, OUTPUT);
        pinMode(COLUMN7, OUTPUT);
        pinMode(ROW_GREEN_1, OUTPUT);
        pinMode(ROW_GREEN_2, OUTPUT);
        pinMode(ROW_GREEN_3, OUTPUT);
        pinMode(ROW_GREEN_4, OUTPUT);
        pinMode(ROW_GREEN_5, OUTPUT);
        pinMode(ROW_RED_1, OUTPUT);
        pinMode(ROW_RED_2, OUTPUT);
        pinMode(ROW_RED_3, OUTPUT);
        pinMode(ROW_RED_4, OUTPUT);
        pinMode(ROW_RED_5, OUTPUT);
        digitalWrite(COLUMN1, LOW);
        digitalWrite(COLUMN2, LOW);
        digitalWrite(COLUMN3, LOW);
        digitalWrite(COLUMN4, LOW);
        digitalWrite(COLUMN5, LOW);
        digitalWrite(COLUMN6, LOW);
        digitalWrite(COLUMN7, LOW);
        digitalWrite(ROW_GREEN_1, LOW);
        digitalWrite(ROW_GREEN_2, LOW);
        digitalWrite(ROW_GREEN_3, LOW);
        digitalWrite(ROW_GREEN_4, LOW);
        digitalWrite(ROW_GREEN_5, LOW);
        digitalWrite(ROW_RED_1, LOW);
        digitalWrite(ROW_RED_2, LOW);
        digitalWrite(ROW_RED_3, LOW);
        digitalWrite(ROW_RED_4, LOW);
        digitalWrite(ROW_RED_5, LOW);
    }

    // Override setState to update the current state
    void setState(State state) override {
        currentState = state;
    }

    void activateColumn(uint8_t column) {
        // reversed compared to schematic
        digitalWrite(COLUMN1, (column == 6) ? HIGH : LOW);
        digitalWrite(COLUMN2, (column == 5) ? HIGH : LOW);
        digitalWrite(COLUMN3, (column == 4) ? HIGH : LOW);
        digitalWrite(COLUMN4, (column == 3) ? HIGH : LOW);
        digitalWrite(COLUMN5, (column == 2) ? HIGH : LOW);
        digitalWrite(COLUMN6, (column == 1) ? HIGH : LOW);
        digitalWrite(COLUMN7, (column == 0) ? HIGH : LOW);
    }

    // Render the current state on the LED matrix
    void renderState(uint32_t Time) override {
        activateColumn(currentColumn);

        switch(currentState) {
            case SHOW:
                digitalWrite(ROW_GREEN_1, currentColumn == 4?HIGH:LOW);
                break;
            case RECOGNISE:
                digitalWrite(ROW_GREEN_1, currentColumn == 5?HIGH:LOW);
                break;
            case EXAM:
                digitalWrite(ROW_GREEN_1, currentColumn == 6?HIGH:LOW);
                break;
            case MEMORIZE:
                digitalWrite(ROW_GREEN_1, currentColumn == 3?HIGH:LOW);
                break;
            case PLAYBACK:
                digitalWrite(ROW_GREEN_1, currentColumn == 4?HIGH:LOW);
                break;
        }
        if (morsePattern && currentColumn < 5) {
            digitalWrite(ROW_GREEN_5, morsePattern[currentColumn] == '-'? HIGH: LOW);
            digitalWrite(ROW_RED_5, morsePattern[currentColumn] == '.'? HIGH: LOW);
        } else {
            digitalWrite(ROW_GREEN_5, LOW);
            digitalWrite(ROW_RED_5, LOW);
        
        }
        digitalWrite(ROW_RED_3, (currentColumn == 6 && morsePixelState)? HIGH: LOW);
        currentColumn = (currentColumn + 1) % 7; // Cycle through columns
    }

    // Override setLetter to update the enabled state of a letter
    void setLetter(char letter, bool enabled) override {
        // Ensure the letter is uppercase
        if (letter >= 'a' && letter <= 'z') {
            letter -= 'a'; // Convert to uppercase index
        } else if (letter >= 'A' && letter <= 'Z') {
            letter -= 'A'; // Convert to index
        } else {
            Serial.println("Invalid letter. Must be A-Z or a-z.");
            return;
        }

        // Update the corresponding index in the letters array
        letters[letter] = enabled;
    }

    // Override setMorsePixel to control the Morse pixel (on or off)
    void setMorsePixel(bool on) override {
        morsePixelState = on; // Store the state of the Morse pixel
    }

    // Override setMorsePattern to set the Morse pattern
    void setMorsePattern(const char *pattern) override {
        morsePattern = pattern;
    }
};
#else

class SerialDisplay : public StateVisualizer {
private:
    bool letters[26];   // Array to store the enabled state of each letter (A-Z)
    State currentState; // Stores the current state (enum)
    char currentLetter; // Stores the current letter
    bool morsePixelState; // Stores the current state of the Morse pixel (on or off)

public:
    // Constructor
    SerialDisplay() : currentState(ROOT), currentLetter(' '), morsePixelState(false) {
        Serial.begin(9600);
    }

    // Override setState to update the current state
    void setState(State state) override {
        currentState = state;
        Serial.print("State changed to: ");
        switch (currentState) {
            case ROOT:
                Serial.println("ROOT");
                break;
            case SHOW:
                Serial.println("SHOW");
                break;
            case RECOGNISE:
                Serial.println("RECOGNISE");
                break;
            case EXAM:
                Serial.println("EXAM");
                break;
            case MEMORIZE:
                Serial.println("MEMORIZE");
                break;
            case PLAYBACK:
                Serial.println("PLAYBACK");
                break;
        }
    }

    // Override renderState to render the current state
    void renderState(uint32_t Time) override {
    }

    // Override setLetter to update the current letter and its enabled state
    void setLetter(char letter, bool enabled) override {
        uint8_t letter_idx;
        // Ensure the letter is uppercase
        if (letter >= 'a' && letter <= 'z') {
            letter_idx = (uint8_t)letter-'a'; // Convert to uppercase index
        } else if (letter >= 'A' && letter <= 'Z') {
            letter_idx = (uint8_t) letter-'A'; // Convert to index
        } else {
            Serial.println("Invalid letter. Must be A-Z or a-z.");
            return;
        }
        if (letters[letter_idx] != enabled) {
            Serial.print("Letter ");
            Serial.print(currentLetter);
            Serial.print(" is now ");
            Serial.println(enabled ? "enabled" : "disabled");

            // Update the corresponding index in the letters array
            letters[letter_idx] = enabled;
        }
    }

    // Override setMorsePixel to control the Morse pixel (on or off)
    void setMorsePixel(bool on) override {
        if (morsePixelState != on) {
            Serial.print("Morse pixel is now ");
            Serial.println(on ? "ON" : "OFF");
            morsePixelState = on;
        }
    }

    // Override setMorsePattern to set the Morse pattern
    void setMorsePattern(const char *pattern) override {
        Serial.print("Morse pattern set to: ");
        Serial.println(pattern);
    }
};
#endif //ifdef AVR_ATmega88




// Global instance of MorseLittleProfessor
StateVisualizer* visualizer;
MorseLittleProfessor* professor;

// Static arrays to store the previous state of the buttons and the time of the last BUTTONDOWN event
static bool previousButtonStates[4] = {false, false, false, false};
static uint32_t buttonDownTimes[4] = {0, 0, 0, 0};

void setup() {

#ifdef __AVR_ATmega88__
    visualizer = new LedMatrixDisplay();
#else
    visualizer = new SerialDisplay();
#endif
    professor = new MorseLittleProfessor(*visualizer);

    // Set pin modes for SW1-SW4 as INPUT with pull-up resistors
    pinMode(SW1, INPUT_PULLUP);
    pinMode(SW2, INPUT_PULLUP);
    pinMode(SW3, INPUT_PULLUP);
    pinMode(SW4, INPUT_PULLUP);
    visualizer->setState(SHOW);
#ifdef __AVR_ATmega88__
    previousButtonStates[0] = digitalRead(SW1);
    previousButtonStates[1] = digitalRead(SW2);
    previousButtonStates[2] = digitalRead(SW3);
    previousButtonStates[3] = digitalRead(SW4);
#endif

}

bool serialbutton(int button) {
    static uint32_t lastReceivedTime[4] = {0, 0, 0, 0}; // Tracks the last time a character was received for each button
    char receivedChar;
    uint32_t currentTime = millis();

    // Check if a character is available on the serial port
    if (Serial.available() > 0) {
        receivedChar = Serial.read(); // Read the character from the serial port

        // Update the last received time based on the character
        switch (receivedChar) {
            case 'A': // Button 0
            case 'a': // Button 0
                lastReceivedTime[0] = currentTime;
                break;
            case 'B': // Button 1
            case 'b': // Button 1
                lastReceivedTime[1] = currentTime;
                break;
            case 'C': // Button 2
            case 'c': // Button 2
                lastReceivedTime[2] = currentTime;
                break;
            case 'D': // Button 3
            case 'd': // Button 3
                lastReceivedTime[3] = currentTime;
                break;
        }
    }

    // Check if the button argument matches and if the last received time is within 400ms
    if (currentTime - lastReceivedTime[button] <= 400) {
        return true; // Button is considered pressed
    }

    return false; // Button is not pressed
}

uint32_t lastButtonScan;

void loop() {
    uint32_t now = millis();
    // Array to store the current state of the but
    if (now > lastButtonScan + 100)
    {
        bool currentButtonStates[4] = {
#ifdef __AVR_ATmega88__
            digitalRead(SW1),
            digitalRead(SW2),
            digitalRead(SW3),
            digitalRead(SW4)
#else
            serialbutton(0),
            serialbutton(1),
            serialbutton(2),
            serialbutton(3)
#endif // ATmega88
        };

        // Check for changes in button states
        for (int i = 0; i < 4; i++) {
            if (currentButtonStates[i] != previousButtonStates[i]) {
                // Determine the event type based on the button state change
                EventType event = currentButtonStates[i] ? BUTTONDOWN : BUTTONUP;
#ifndef __AVR_ATmega88__
            Serial.print("Letter ");
            Serial.print(i);
            Serial.print(" is now ");
            Serial.println(currentButtonStates[i]? "enabled" : "disabled");
#endif
                if (event == BUTTONDOWN) {
                    professor->handle(BUTTONDOWN, static_cast<ButtonId>(i), 0, now);
                    // Record the time of the BUTTONDOWN event
                    buttonDownTimes[i] = now;
                } else {
                    // BUTTONUP event
                    uint32_t buttonTime = now - buttonDownTimes[i];
                    professor->handle(BUTTONUP, static_cast<ButtonId>(i), buttonTime, now);
                }
            }
        }

        lastButtonScan = now;
        // Update the previous button states
        for (int i = 0; i < 4; i++) {
            previousButtonStates[i] = currentButtonStates[i];
        }
    }

    // Call the handle method with the TICK event type
    professor->handle(TICK, BUTTON1, 0, now);

}
