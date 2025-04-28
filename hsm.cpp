#include "hsm.h"

void Hsm::dispatch(Event& event) {

    if (currentState) {
        HandleResult result = currentState->handle(event);
        if (result.type == HandleResult::TRANSITION) {
            Event exitEvent(Event::EXIT);
            currentState->handle(exitEvent);
            currentState = result.data.nextState;
            Event enterEvent(Event::ENTER);
            currentState->handle(enterEvent);
        }
    }
}