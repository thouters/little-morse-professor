#include "Arduino.h"

#include "professor.h"


#define NUMBER_OF_LETTERS (26)
#ifdef __AVR_ATmega88__
#define NUMBER_OF_COLUMNS (7)
#define NUMBER_OF_ROWS (5)

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
    bool letters[NUMBER_OF_LETTERS];   // Array to store the enabled state of each letter (A-Z)
    bool morsePixelState; // Stores the current state of the Morse pixel (on or off)
    uint8_t currentColumn;
    const char *morsePattern; // Stores the current Morse pattern
    int symbolIndex;
    StateIndicator currentState; // Stores the current state (enum)

public:
    // Constructor
    LedMatrixDisplay() : currentState(SHOW), morsePixelState(false), currentColumn(0), morsePattern(nullptr), symbolIndex(-1) {
        // Initialize all letters to false (disabled)
        for (int i = 0; i < NUMBER_OF_LETTERS; i++) {
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
    void setState(StateIndicator state) override {
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

        // TODO: optimize access by directly accessing AVR PORTC
        activateColumn(0xff); // clear all columns
        // layout of ROW 1-4 is to show all letters from the letters array
        digitalWrite(ROW_GREEN_1, letters[currentColumn+0*NUMBER_OF_COLUMNS] ? HIGH : LOW);
        digitalWrite(ROW_GREEN_2, letters[currentColumn+1*NUMBER_OF_COLUMNS] ? HIGH : LOW);
        digitalWrite(ROW_GREEN_3, letters[currentColumn+2*NUMBER_OF_COLUMNS] ? HIGH : LOW);
        if (currentColumn >= NUMBER_OF_LETTERS - 3 * NUMBER_OF_COLUMNS) {
            // if the current column is out of bounds for the alphabet, turn off the row
            digitalWrite(ROW_GREEN_4, LOW);
        } else {
            digitalWrite(ROW_GREEN_4, letters[currentColumn+3*NUMBER_OF_COLUMNS] ? HIGH : LOW);
        }
//        digitalWrite(ROW_RED_4, (currentColumn == 6 && morsePixelState)? HIGH: LOW); // FIXME

        // last row
        if (currentColumn < 4) {
            if (morsePattern) {
                if (morsePixelState && currentColumn == symbolIndex) {
                    if (morsePattern[currentColumn] != '\0') {
                        digitalWrite(ROW_GREEN_5, LOW);
                        digitalWrite(ROW_RED_5, LOW);
                    } else {
                        // cursor in enter mode
                        digitalWrite(ROW_GREEN_5, HIGH);
                        digitalWrite(ROW_RED_5, HIGH);
                    }
                } else {
                    
                    digitalWrite(ROW_GREEN_5, morsePattern[currentColumn] == '-'? HIGH: LOW);
                    digitalWrite(ROW_RED_5, morsePattern[currentColumn] == '.'? HIGH: LOW);
                }
            } else {
                digitalWrite(ROW_GREEN_5, LOW);
                digitalWrite(ROW_RED_5, LOW);
            }
        }  else {
            switch(currentState) {
                case SHOW:
                    digitalWrite(ROW_GREEN_5, currentColumn == 5?HIGH:LOW);
                    break;
                case RECOGNISE:
                    digitalWrite(ROW_GREEN_5, currentColumn == 6?HIGH:LOW);
                    break;
            }
            digitalWrite(ROW_RED_5, LOW);
        }
        activateColumn(currentColumn);
        currentColumn = (currentColumn + 1) % 7; // Cycle through columns
    }

    // Override setLetter to update the enabled state of a letter
    void setLetter(char letter, bool enabled) override {
        // Convert to index
        if (letter >= 'a' && letter <= 'z') {
            letter -= 'a'; 
        } else if (letter >= 'A' && letter <= 'Z') {
            letter -= 'A';
        } else {
            Serial.println("Invalid letter. Must be A-Z or a-z.");
            return;
        }

        // Update the corresponding index in the letters array
        letters[letter] = enabled;
    }

    // Override setMorsePixel to control the Morse pixel (on or off)
    void setMorsePixel(bool on, int newSymbolIndex) override {
        morsePixelState = on; // Store the state of the Morse pixel
        symbolIndex = newSymbolIndex;
        // Add logic to handle symbolIndex if needed
    }

    // Override setMorsePattern to set the Morse pattern
    void setMorsePattern(const char *pattern) override {
        morsePattern = pattern;
    }
};
#else

class SerialDisplay : public StateVisualizer {
private:
    bool letters[NUMBER_OF_LETTERS];   // Array to store the enabled state of each letter (A-Z)
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
            case QUIZ:
                Serial.println("QUIZ");
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
    void setMorsePixel(bool on, int symbolIndex) override {
        if (morsePixelState != on) {
            Serial.print("Morse pixel is now ");
            Serial.println(on ? "ON" : "OFF");
            morsePixelState = on;
        }
        // Add logic to handle symbolIndex if needed
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
    professor->begin();

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

// Replace EventType and its usage with the Event tagged union structure
// Update the loop function to use the Event structure
void loop() {
    uint32_t now = millis();
    if (now > lastButtonScan + 100) {
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
#endif
        };

        for (int i = 0; i < 4; i++) {
            if (currentButtonStates[i] != previousButtonStates[i]) {
                Event event(currentButtonStates[i] ? Event::BUTTONDOWN : Event::BUTTONUP);
                event.data.buttonData = {static_cast<ButtonId>(i), buttonDownTimes[i], now};

#ifndef __AVR_ATmega88__
                Serial.print("Button ");
                Serial.print(i);
                Serial.print(" is now ");
                Serial.println(currentButtonStates[i] ? "enabled" : "disabled");
#endif

                event.data.buttonData.buttonTime = now - buttonDownTimes[i];
                buttonDownTimes[i] = now;
                professor->dispatch(event);
            }
        }

        lastButtonScan = now;
        for (int i = 0; i < 4; i++) {
            previousButtonStates[i] = currentButtonStates[i];
        }
    }

    Event tickEvent(Event::TICK);
    tickEvent.data.tickData = {now};
    professor->dispatch(tickEvent);
}
