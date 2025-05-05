#include "hsm.h"

void Hsm::dispatch(Event& event) {

    if (currentState) {
        HandleResult result = currentState->handle(event);
        // switch instead of if:
        if (result.type == HandleResult::HANDLED) {
            return; // Event handled, no further action needed
        } else if (result.type == HandleResult::PARENT) {
//            result = currentState->parentState()->handle(event);
            return; // Event not handled, but no transition needed
        } else if (result.type == HandleResult::TRANSITION) {
            Event exitEvent = Event::exit(event.data.tickData.time);
            currentState->handle(exitEvent);
            currentState = result.data.nextState;
            Event enterEvent = Event::enter(event.data.tickData.time);
            currentState->handle(enterEvent);
        }
    }
}