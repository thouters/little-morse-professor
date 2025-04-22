#pragma once
#include <stdint.h>
#include <stdbool.h>
#define DotTime 500
#define DashTime 1000
#define ShortPauseTime 500
#define LongPauseTime 2000

enum EventType {
    ENTER,
    EXIT,
    TICK,
    BUTTONDOWN,
    BUTTONUP
};

enum ButtonId {
    BUTTON1,    // letter selection button
    BUTTON2,    // confirm/skip button
    BUTTON3,    // morse code input button
    BUTTON4     // mode selection button
};
#define BUTTON_SELECT_LETTER BUTTON1
#define BUTTON_SKIP BUTTON1
#define BUTTON_CONFIRM BUTTON2
#define BUTTON_MORSE_INPUT BUTTON3
#define BUTTON_MODE_SELECT BUTTON4



enum State {
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
    virtual void setState(State state) = 0;

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

class ShowState {
private:
    char currentLetter;          // Current letter being processed
    MorseLittleProfessor* morseLittleProfessor = nullptr;
public: 
    bool handle(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time);
    void begin(MorseLittleProfessor& pMorseLittleProfessor);
    State parent();
};

class MorseLittleProfessor {
private:
    State currentState;          // Current state of the application
    uint32_t lastChangeTime = 0; // Time of the last change in Morse pixel state
    bool morsePixelOn = false;   // State of the Morse pixel (on or off)
    int currentPatternIndex = 0; // Index of the current symbol in the Morse pattern
    ShowState showState; // Instance of the ShowState class

    // State-specific handle methods
    bool handleRoot(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time);

public:
    StateVisualizer& visualizer; // Reference to the StateVisualizer instance
    const char* currentLetterPattern; // Pointer to the Morse pattern for the current letter
    // Constructor
    MorseLittleProfessor(StateVisualizer& vis) : visualizer(vis), currentState(ROOT), currentLetterPattern(nullptr) {
        // Initialize the visualizer
    }
    void begin();
    void lookupMorsePattern(char letter);
    void setState(State state, uint32_t Time);

    void handle(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time);

    void startMorsePattern(uint32_t Time);
    bool updateMorsePixel(uint32_t newTime) ;
    void setOnlyLetter(char letter) ;

};
