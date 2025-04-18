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
    BUTTON1,
    BUTTON2,
    BUTTON3,
    BUTTON4
};



enum State {
    ROOT,
    SHOW,
    RECOGNISE,
    EXAM,
    MEMORIZE,
    PLAYBACK,
    MAX_STATE
};

// Abstract class to visualize the state of the application
class StateVisualizer {
public:
    // Virtual destructor for proper cleanup of derived classes
    virtual ~StateVisualizer() {}

    // Method to set the state (enum values: show, exam, memorize, playback)
    virtual void setState(State state) = 0;

    // Method to render the current state
    virtual void renderState(uint32_t Time) = 0;

    // Method to set the current letter and its enabled state
    virtual void setLetter(char letter, bool enabled) = 0;

    // Method to set the Morse pixel (on or off)
    virtual void setMorsePixel(bool on) = 0;

    // Method to set the Morse pattern
    virtual void setMorsePattern(const char *pattern) = 0;
};