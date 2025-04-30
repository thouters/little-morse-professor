#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "hsm.h"
#define DotTime 500
#define DashTime 1000
#define ShortPauseTime 500
#define LongPauseTime 2000


#define BUTTON_SELECT_LETTER BUTTON1
#define BUTTON_SKIP BUTTON1
#define BUTTON_CONFIRM BUTTON2
#define BUTTON_MORSE_INPUT BUTTON3
#define BUTTON_MODE_SELECT BUTTON4



enum StateIndicator {
    ROOT,
    SHOW,
    RECOGNISE,
    QUIZ,
    MEMORIZE,
    PLAYBACK,
    MAX_STATE
};


// Abstract class to visualize the state of the application
class StateVisualizer {
public:
    // Virtual destructor for proper cleanup of derived classes
    virtual ~StateVisualizer() {}

    // Method to set the state (enum values: show, quiz, memorize, playback)
    virtual void setState(StateIndicator state) = 0;

    // Method to render the current state
    virtual void renderState(uint32_t Time) = 0;

    // Method to set the current letter and its enabled state
    virtual void setLetter(char letter, bool enabled) = 0;

    // Method to set the Morse pixel (on or off)
    virtual void setMorsePixel(bool on, int symbolIndex) = 0;

    // Method to set the Morse pattern
    virtual void setMorsePattern(const char *pattern) = 0;
};

class MorseLittleProfessor;

class ShowState: public HsmState {
private:
    char currentLetter;          // Current letter being processed
    MorseLittleProfessor* morseLittleProfessor = nullptr;
public: 
    void begin(MorseLittleProfessor& pMorseLittleProfessor);
    HandleResult_t handle(Event& event);
};

class RecogniseState: public HsmState {
private:
    uint32_t markCounter = 0;
    uint32_t markTimes[32];     // Array to store the down times of button presses
    uint32_t spaceTimes[32];     // Array to store the up times of button releases 
    uint32_t lastButtonPressTime = 0; // Time of the last button press: to autoreset
    char morsePattern[32]; // Array to store the Morse pattern
    MorseLittleProfessor* morseLittleProfessor = nullptr;
public: 
    void begin(MorseLittleProfessor& pMorseLittleProfessor);
    HandleResult_t handle(Event& event);
    void evaluateInput(void);
};

// both the Hsm rootstate and statemachine at once.
class MorseLittleProfessor : public Hsm, public HsmState {
private:
    uint32_t lastChangeTime = 0; // Time of the last change in Morse pixel state
    bool morsePixelOn = false;   // State of the Morse pixel (on or off)
    int currentPatternIndex = 0; // Index of the current symbol in the Morse pattern

public:
    ShowState showState; 
    RecogniseState recogniseState; 
    StateVisualizer& visualizer; // Reference to the StateVisualizer instance
    const char* currentLetterPattern = ""; // Pointer to the Morse pattern for the current letter
    // Constructor
    MorseLittleProfessor(StateVisualizer& vis) : visualizer(vis), currentLetterPattern(nullptr) {
        // Initialize the visualizer
    }
    void begin();
    const char * lookupMorsePattern(char letter);

    void startMorsePattern(uint32_t Time);
    bool updateMorsePixel(uint32_t newTime) ;
    void setOnlyLetter(char letter) ;
    HandleResult_t handle(Event& event);
};
