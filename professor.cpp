#include "professor.h"
#include "morsetable.h"

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
    bool handleRoot(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time) {
        switch (event) {
            case ENTER:
                break;
            case EXIT:
                return true;
            case TICK:
                visualizer.renderState(Time);
                return true;
            case BUTTONDOWN:
                switch(buttonId )   {
                    case BUTTON4:
                        State nextState  = static_cast<State>((currentState + 1) % 3);
                        if (nextState == ROOT) {
                            nextState = static_cast<State>(nextState + 1);
                        }
                        setState(nextState, Time);
                        return true;
                }
                break;
            case BUTTONUP:
                break;
        }
        return false;
    }
    void setOnlyLetter(char letter) {
        for (char i = 'A'; i < 'Z'+1; i++) {
            if (i == letter) {
                visualizer.setLetter(letter, true);
            } else {
                visualizer.setLetter(i, false);
            }
        }
    }

    bool handleShow(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time) {
        switch (event) {
            case ENTER:
                currentLetter = 'A';
                lookupMorsePattern(); 
                startMorsePattern(Time);
                visualizer.setLetter(currentLetter, true);
                return true;
            case EXIT:
                break;
            case TICK:
                if (updateMorsePixel(Time)) {
                    // pattern done, repeat it
                    lookupMorsePattern(); // Get the Morse pattern for the current letter
                    startMorsePattern(Time);
                }
                visualizer.renderState(Time);
                return true;
            case BUTTONDOWN:
                switch (buttonId) {
                    case BUTTON1:

                        // Cycle through letters
                        currentLetter++;
                        if (currentLetter > 'Z') {
                            currentLetter = 'A';
                        }
                        lookupMorsePattern(); // Update the Morse pattern for the new letter
                        startMorsePattern(Time);
                        setOnlyLetter(currentLetter);
                        return true;
                    case BUTTON2:
                        break;
                    case BUTTON3:
                        break;
                    case BUTTON4:
                        break;
                }
                break;
            case BUTTONUP:
                break;
        }
        return false;
    }

    bool handleExam(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time) {
        switch (event) {
            case ENTER:
                break;
            case EXIT:
                break;
            case TICK:
                visualizer.renderState(Time);
                return true;
                break;
            case BUTTONDOWN:
                break;
            case BUTTONUP:
                break;
        }
        return false;
    }

    bool handleMemorize(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time) {
        switch (event) {
            case ENTER:
                break;
            case EXIT:
                break;
            case TICK:
                visualizer.renderState(Time);
                return true;
            case BUTTONDOWN:
                break;
            case BUTTONUP:
                break;
        }
        return false;
    }

    bool handlePlayback(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time) {
        switch (event) {
            case ENTER:
                break;
            case EXIT:
                break;
            case TICK:
                visualizer.renderState(Time);
                return true;
            case BUTTONDOWN:
                break;
            case BUTTONUP:
                break;
        }
        return false;
    }

    bool handleRecognise(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time) {
        switch (event) {
            case ENTER:
                // Initialize any variables or state for RECOGNISE
                break;
            case EXIT:
                // Cleanup or finalize any state for RECOGNISE
                break;
            case TICK:
                break;
            case BUTTONDOWN:
                break;
            case BUTTONUP:
                break;
        }
        return false;
    }

public:
    // Constructor
    MorseLittleProfessor(StateVisualizer& vis) : visualizer(vis), currentState(ROOT), currentLetter('A'), currentLetterPattern(nullptr) {
        // Initialize the visualizer
        setState(SHOW, 0);
    }

    // Set the current state
    void setState(State state, uint32_t Time) {
        // Trigger EXIT event for the current state
        handle(EXIT, BUTTON1, 0, Time);

        // Update the current state
        currentState = state;

        // Notify the visualizer of the state change
        visualizer.setState(state);

        // Trigger ENTER event for the new state
        handle(ENTER, BUTTON1, 0, Time);
    }

    // General handle method
    void handle(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time) {
        bool handled = false;
        switch (currentState) {
            case ROOT:
                handled = handleRoot(event, buttonId, buttonTime, Time);
                break;
            case SHOW:
                handled = handleShow(event, buttonId, buttonTime, Time);
                break;
            case EXAM:
                handled = handleExam(event, buttonId, buttonTime, Time);
                break;
            case MEMORIZE:
                handled = handleMemorize(event, buttonId, buttonTime, Time);
                break;
            case PLAYBACK:
                handled = handlePlayback(event, buttonId, buttonTime, Time);
                break;
            case RECOGNISE: // New state
                handled = handleRecognise(event, buttonId, buttonTime, Time);
                break;
        }
        if (!handled) {
            handled = handleRoot(event, buttonId, buttonTime, Time);
        }
    }

    // Method to look up the Morse pattern for the current letter
    void lookupMorsePattern() {
        if (currentLetter < 'A' || currentLetter > 'Z') {
            currentLetterPattern = nullptr;
            return;
        }

        // Calculate the index in the Morse table
        int index = currentLetter - 'A' + 33; // Offset for ASCII table

#ifdef ARDUINO
        // Read the pointer to the string from PROGMEM
        const char* morsePtr = (const char*)pgm_read_ptr(&morsecode_table[index]);
        // Copy the string from PROGMEM to a buffer
        static char buffer[10]; // Temporary buffer to store the pattern
        for (int i = 0; i < sizeof(buffer);i++) {
            buffer[i] = 0;
        }
        strcpy_P(buffer, morsePtr);
        currentLetterPattern = buffer; // skip the character's ascii value at index 0
#else
        currentLetterPattern = (const char*)morsecode_table[index];
#endif
        visualizer.setMorsePattern(currentLetterPattern);

    }
    void startMorsePattern(uint32_t Time) {
        lastChangeTime = Time;
        currentPatternIndex = 0; // Reset the pattern index
        morsePixelOn = true; // Set the pixel to ON
        visualizer.setMorsePixel(true, currentPatternIndex); // Turn on the pixel

    }

    // Method to update the Morse pixel based on the current pattern and time
    bool updateMorsePixel(uint32_t newTime) {
        if (!currentLetterPattern) {
            // No pattern, turn off the pixel
            visualizer.setMorsePixel(false, currentPatternIndex); // Updated to pass symbolIndex
            return true;
        }

        if (morsePixelOn) {
            // If the pixel is currently ON, check if it's time to turn it OFF
            uint32_t duration = (currentLetterPattern[currentPatternIndex] == '-') ? DashTime : DotTime;
            if (newTime - lastChangeTime >= duration) {
                visualizer.setMorsePixel(false, currentPatternIndex); // Updated to pass symbolIndex
                morsePixelOn = false;
                lastChangeTime = newTime;
            }
        } else {
            // If the pixel is OFF, check if it's time to move to the next symbol or start a pause
            if (currentLetterPattern[currentPatternIndex] != '\0') {
                if (newTime - lastChangeTime >= ShortPauseTime) {
                    // Move to the next symbol in the pattern
                    currentPatternIndex++;
                    if (currentLetterPattern[currentPatternIndex] != '\0') {
                        lastChangeTime = newTime;
                        visualizer.setMorsePixel(true, currentPatternIndex); // Turn on the pixel
                        morsePixelOn = true;
                    }
                    lastChangeTime = newTime;
                }
            } else {
                if (newTime - lastChangeTime >= LongPauseTime) {
                    // Move to the next symbol in the pattern
                    return true;
                }
            }
        }
        return false; // No change in state
    }
};
