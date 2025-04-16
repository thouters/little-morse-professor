#include "Arduino.h"
#include "morsetable.h"

#define DotTime 200
#define DashTime 800
#define ShortPauseTime 1000


#define SW3 PIN_PC0
#define GC1 PIN_PC1 
#define GC2 PIN_PC2 
#define GC3 PIN_PC3 
#define GC4 PIN_PC4 
#define GC5 PIN_PC5 
#define ROW1 PIN_PD0
#define ROW2 PIN_PD1
#define ROW3 PIN_PD2
#define ROW4 PIN_PD3
#define ROW5 PIN_PD4 
#define ROW6 PIN_PD5
#define ROW7 PIN_PD6
#define SW4 PIN_PD7
#define SW1 PIN_PB0 
#define SPKR PIN_PB1 
#define SW2 PIN_PB2
#define MOSI_RC1 PIN_PB3
#define MISO_RC2 PIN_PB4 
#define SCK_RC3 PIN_PB5 
#define RC4 PIN_PB6
#define RC5 PIN_PB7

enum State {
    ROOT,
    SHOW,
    EXAM,
    MEMORIZE,
    PLAYBACK
};

// Abstract class to visualize the state of the application
class StateVisualizer {
public:
    // Virtual destructor for proper cleanup of derived classes
    virtual ~StateVisualizer() {}

    // Method to set the state (enum values: show, exam, memorize, playback)
    virtual void setState(State state) = 0;

    // Method to render the current state
    virtual void renderState() = 0;

    // Method to set the current letter and its enabled state
    virtual void setLetter(char letter, bool enabled) = 0;

    // Method to set the Morse pixel (on or off)
    virtual void setMorsePixel(bool on) = 0;
};

/* Render teh application state on a dot matrix display.
* The display has 7 rows and 5 columns.
* The physical device is built so that the vertical lines are the rows,
* and the horizontal lines are columns (mindfuck)
* so we will have vertical scanlines
*/
class LedMatrixDisplay : public StateVisualizer {
private:
    State currentState; // Stores the current state (enum)
    bool letters[26];   // Array to store the enabled state of each letter (A-Z)
    bool morsePixelState; // Stores the current state of the Morse pixel (on or off)
    uint8_t currentRow;
public:
    // Constructor
    LedMatrixDisplay() : currentState(SHOW), morsePixelState(false) {
        // Initialize all letters to false (disabled)
        for (int i = 0; i < 26; i++) {
            letters[i] = false;
        }
        currentRow = 0; // Initialize the current column

    }

    // Override setState to update the current state
    void setState(State state) override {
        currentState = state;
        Serial.print("changing state: ");
        switch (currentState) {
            case SHOW:
                Serial.println("SHOW");
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
    void activateRow(uint8_t column) {
        digitalWrite(ROW1, (column == 1)? HIGH:LOW);
        digitalWrite(ROW2, (column == 2)? HIGH:LOW);
        digitalWrite(ROW3, (column == 3)? HIGH:LOW);
        digitalWrite(ROW4, (column == 4)? HIGH:LOW);
        digitalWrite(ROW5, (column == 5)? HIGH:LOW);
        digitalWrite(ROW6, (column == 6)? HIGH:LOW);
        digitalWrite(ROW7, (column == 7)? HIGH:LOW);
    }

    // Render the current state on the LED matrix
    void renderState() override {
        activateRow(currentRow); 
        digitalWrite(GC1, HIGH);

        currentRow = (currentRow + 1) % 7; // Cycle through columns
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

};

enum EventType {
    ENTER,
    EXIT,
    TICK,
    BUTTONDOWN,
    BUTTONUP
};

enum ButtonId {
    BUTTON1,
    BUTTON2,
    BUTTON3,
    BUTTON4
};

class MorseLittleProfessor {
private:
    StateVisualizer& visualizer; // Reference to the StateVisualizer instance
    State currentState;          // Current state of the application
    char currentLetter;          // Current letter being processed
    const char* currentLetterPattern; // Pointer to the Morse pattern for the current letter
    uint32_t lastChangeTime = 0; // Time of the last change in Morse pixel state
    bool morsePixelOn = false;   // State of the Morse pixel (on or off)
    int currentPatternIndex = 0; // Index of the current symbol in the Morse pattern

    // State-specific handle methods
    void handleRoot(EventType event, ButtonId buttonId, uint32_t buttonTime) {
        switch (event) {
            case ENTER:
                setState(SHOW);
                break;
            case EXIT:
                Serial.println("ROOT: EXIT event");
                break;
            case TICK:
                visualizer.renderState();
                break;
            case BUTTONDOWN:
                Serial.println("ROOT: BUTTONDOWN event");
                break;
            case BUTTONUP:
                Serial.println("ROOT: BUTTONUP event");
                break;
        }
    }

    void handleShow(EventType event, ButtonId buttonId, uint32_t Time) {
        switch (event) {
            case ENTER:
                currentLetter = 'A';
                lookupMorsePattern(); // Get the Morse pattern for the current letter
                break;
            case EXIT:
                Serial.println("SHOW: EXIT event");
                break;
            case TICK:
                if (updateMorsePixel(Time))
                {
                    lookupMorsePattern(); // Get the Morse pattern for the current letter
                }
                visualizer.renderState();
                break;
            case BUTTONDOWN:
                switch (buttonId) {
                    case BUTTON1:
                        // Cycle through letters
                        currentLetter++;
                        if (currentLetter > 'Z') {
                            currentLetter = 'A';
                        }
                        lookupMorsePattern(); // Update the Morse pattern for the new letter
                        for (int i = 0; i < 26; i++) {
                            if (i == currentLetter - 'A') {
                                visualizer.setLetter(currentLetter, true);
                            } else {
                                visualizer.setLetter('A' + i, false);
                            }
                        }
                        break;
                    case BUTTON2:
                        Serial.println("SHOW: BUTTON2 event");
                        break;
                    case BUTTON3:
                        Serial.println("SHOW: BUTTON3 event");
                        break;
                    case BUTTON4:
                        Serial.println("SHOW: BUTTON4 event");
                        break;
                }
                break;
            case BUTTONUP:
                Serial.println("SHOW: BUTTONUP event");
                break;
        }
    }

    void handleExam(EventType event, ButtonId buttonId, uint32_t buttonTime) {
        switch (event) {
            case ENTER:
                Serial.println("EXAM: ENTER event");
                break;
            case EXIT:
                Serial.println("EXAM: EXIT event");
                break;
            case TICK:
                visualizer.renderState();
                break;
            case BUTTONDOWN:
                Serial.println("EXAM: BUTTONDOWN event");
                break;
            case BUTTONUP:
                Serial.println("EXAM: BUTTONUP event");
                break;
        }
    }

    void handleMemorize(EventType event, ButtonId buttonId, uint32_t buttonTime) {
        switch (event) {
            case ENTER:
                Serial.println("MEMORIZE: ENTER event");
                break;
            case EXIT:
                Serial.println("MEMORIZE: EXIT event");
                break;
            case TICK:
                visualizer.renderState();
                break;
            case BUTTONDOWN:
                Serial.println("MEMORIZE: BUTTONDOWN event");
                break;
            case BUTTONUP:
                Serial.println("MEMORIZE: BUTTONUP event");
                break;
        }
    }

    void handlePlayback(EventType event, ButtonId buttonId, uint32_t buttonTime) {
        switch (event) {
            case ENTER:
                Serial.println("PLAYBACK: ENTER event");
                break;
            case EXIT:
                Serial.println("PLAYBACK: EXIT event");
                break;
            case TICK:
                visualizer.renderState();
                break;
            case BUTTONDOWN:
                Serial.println("PLAYBACK: BUTTONDOWN event");
                break;
            case BUTTONUP:
                Serial.println("PLAYBACK: BUTTONUP event");
                break;
        }
    }

public:
    // Constructor
    MorseLittleProfessor(StateVisualizer& vis) : visualizer(vis), currentState(ROOT), currentLetter('A'), currentLetterPattern(nullptr) {}

    // Set the current state
    void setState(State state) {
        // Trigger EXIT event for the current state
        handle(EXIT, BUTTON1, 0);

        // Update the current state
        currentState = state;

        // Notify the visualizer of the state change
        visualizer.setState(state);

        // Trigger ENTER event for the new state
        handle(ENTER, BUTTON1, 0);
    }

    // General handle method
    void handle(EventType event, ButtonId buttonId, uint32_t buttonTime) {
        switch (currentState) {
            case ROOT:
                handleRoot(event, buttonId, buttonTime);
                break;
            case SHOW:
                handleShow(event, buttonId, buttonTime);
                break;
            case EXAM:
                handleExam(event, buttonId, buttonTime);
                break;
            case MEMORIZE:
                handleMemorize(event, buttonId, buttonTime);
                break;
            case PLAYBACK:
                handlePlayback(event, buttonId, buttonTime);
                break;
        }
    }

    // Method to look up the Morse pattern for the current letter
    void lookupMorsePattern() {
        if (currentLetter < 'A' || currentLetter > 'Z') {
            Serial.println("Invalid letter for Morse lookup.");
            currentLetterPattern = nullptr;
            return;
        }

        // Calculate the index in the Morse table
        int index = currentLetter - 'A' + 33; // Offset for ASCII table

        // Read the pointer to the string from PROGMEM
        const char* morsePtr = (const char*)pgm_read_ptr(&morsecode_table[index]);

        // Copy the string from PROGMEM to a buffer
        static char buffer[10]; // Temporary buffer to store the pattern
        strcpy_P(buffer, morsePtr);
        currentLetterPattern = buffer;

        // Debug output
        Serial.print("Morse pattern for ");
        Serial.print(currentLetter);
        Serial.print(": ");
        Serial.println(currentLetterPattern);

        currentPatternIndex = 0; // Reset the pattern index
    }

    // Method to update the Morse pixel based on the current pattern and time
    bool updateMorsePixel(uint32_t newTime) {
        if (!currentLetterPattern || currentLetterPattern[currentPatternIndex] == '\0') {
            // No pattern or end of pattern, turn off the pixel
            visualizer.setMorsePixel(false);
            return;
        }

        if (morsePixelOn) {
            // If the pixel is currently ON, check if it's time to turn it OFF
            uint32_t duration = (currentLetterPattern[currentPatternIndex] == '-') ? DashTime : DotTime;
            if (newTime - lastChangeTime >= duration) {
                visualizer.setMorsePixel(false); // Turn off the pixel
                morsePixelOn = false;
                lastChangeTime = newTime;
            }
        } else {
            // If the pixel is OFF, check if it's time to move to the next symbol or start a pause
            if (newTime - lastChangeTime >= ShortPauseTime) {
                // Move to the next symbol in the pattern
                currentPatternIndex++;
                if (currentLetterPattern[currentPatternIndex] != '\0') {
                    // Turn the pixel ON for the next symbol
                    visualizer.setMorsePixel(true);
                    morsePixelOn = true;
                    lastChangeTime = newTime;
                    return true;
                }
            }
        }
        return false; // No change in state
    }
};

// Global instance of MorseLittleProfessor
StateVisualizer* visualizer;
MorseLittleProfessor* professor;

// Static arrays to store the previous state of the buttons and the time of the last BUTTONDOWN event
static bool previousButtonStates[4] = {false, false, false, false};
static uint32_t buttonDownTimes[4] = {0, 0, 0, 0};

void setup() {
    Serial.begin(9600);

    // Create an instance of LedMatrixDisplay and pass it to MorseLittleProfessor
    visualizer = new LedMatrixDisplay();
    professor = new MorseLittleProfessor(*visualizer);

    // Set pin modes for SW1-SW4 as INPUT with pull-up resistors
    pinMode(SW1, INPUT_PULLUP);
    pinMode(SW2, INPUT_PULLUP);
    pinMode(SW3, INPUT_PULLUP);
    pinMode(SW4, INPUT_PULLUP);

}

void loop() {
    // Array to store the current state of the buttons
    bool currentButtonStates[4] = {
        digitalRead(SW1),
        digitalRead(SW2),
        digitalRead(SW3),
        digitalRead(SW4)
    };

    // Check for changes in button states
    for (int i = 0; i < 4; i++) {
        if (currentButtonStates[i] != previousButtonStates[i]) {
            // Determine the event type based on the button state change
            EventType event = currentButtonStates[i] ? BUTTONDOWN : BUTTONUP;
            if (event == BUTTONDOWN) {
                professor->handle(BUTTONDOWN, static_cast<ButtonId>(i), 0);
                // Record the time of the BUTTONDOWN event
                buttonDownTimes[i] = millis();
            } else {
                // BUTTONUP event
                uint32_t buttonTime = millis() - buttonDownTimes[i];
                professor->handle(BUTTONUP, static_cast<ButtonId>(i), buttonTime);
            }
        }
    }

    // Call the handle method with the TICK event type
    professor->handle(TICK, BUTTON1, millis());

    // Update the previous button states
    for (int i = 0; i < 4; i++) {
        previousButtonStates[i] = currentButtonStates[i];
    }

    delay(20); // 20ms delay
}
