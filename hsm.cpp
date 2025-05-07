#include "hsm.h"

void panic()
{
    // Handle panic situation
    while (1) {
        // Infinite loop to indicate a critical error
    }
}

void Hsm::dispatch(Event& event) {

    if (!currentState) {
        panic();
    }

    HandleResult result;

    result = currentState->handle(event);
    while (result.type != HandleResult::HANDLED) {
        if (result.type == HandleResult::PARENT) {
            HsmState * cursor = currentState;
            // the root state has parent pointing to itself
            while (cursor->parentState != cursor) {
                result = cursor->parentState->handle(event);
                if (result.type == HandleResult::PARENT) {
                    cursor = cursor->parentState;
                    if(cursor->parentState == cursor) {
                        // can't go beyond the root state
                        return;
                    }
                } else {
                    break;
                }
            }
        } else if (result.type == HandleResult::TRANSITION) {
            HsmState * newState = result.data.nextState;
            Event exitEvent = Event::exit(event.data.tickData.time);
            Event enterEvent = Event::enter(event.data.tickData.time);

            currentState->handle(exitEvent);
            if (currentState->parentState != newState->parentState) {
                // support only depth 1
                // Todo add panic if needed
                currentState->parentState->handle(exitEvent);
                newState->parentState->handle(enterEvent);
            }

            currentState = newState;
            currentState->handle(enterEvent);
            return;
        }
    }
}