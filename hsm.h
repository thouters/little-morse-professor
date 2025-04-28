#pragma once
#include <stdint.h>
#include <stdbool.h>

//#include "professor.h"

enum ButtonId {
    BUTTON1,    // letter selection button
    BUTTON2,    // confirm/skip button
    BUTTON3,    // morse code input button
    BUTTON4     // mode selection button
};
// Define a tagged union for EventType with associated data
struct Event {
    enum Type {
        HSM_START,
        ENTER,
        EXIT,
        TICK,
        BUTTONDOWN,
        BUTTONUP
    } type;

    union Data {
        struct {
            uint32_t time;
        } tickData;

        struct {
            ButtonId buttonId;
            uint32_t buttonTime;
            uint32_t time;
        } buttonData;

        Data() {} // Default constructor
        ~Data() {} // Destructor
    } data;
    Event(Type t) : type(t) {}
};
typedef struct Event Event_t;

// Define a function prototype for a State handler
struct HandleResult; // Forward declaration
typedef HandleResult (*State_handler_t)(Event &event);
class HsmState ;

// Define a tagged union structure for handling state transitions
struct HandleResult {
    enum ResultType {
        HANDLED,
        PARENT,
        TRANSITION
    } type;

    union {
        HsmState *nextState; // Used when type is TRANSITION
    } data;

    // Factory methods for convenience
    static HandleResult handled() {
        return {HANDLED};
    }

    static HandleResult parent() {
        return {PARENT};
    }

    static HandleResult transition(HsmState *state) {
        HandleResult result;
        result.type = TRANSITION;
        result.data.nextState = state;
        return result;
    }
};
typedef struct HandleResult HandleResult_t;

class HsmState {
public:
    virtual HandleResult_t handle(Event& event) = 0; // Pure virtual function
    virtual ~HsmState() = default; // Virtual destructor for proper cleanup
};


class Hsm {
protected:
    HsmState *currentState;
public:
    void dispatch(Event& event);
    virtual ~Hsm() = default; // Virtual destructor for proper cleanup
};

