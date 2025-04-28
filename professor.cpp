#include "professor.h"
#include "morsetable.h"

#ifndef ARDUINO
// long live printf debugging ^_^
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
using std::cout;
using std::endl;
#endif

void MorseLittleProfessor::begin() {
    showState.begin(*this);
    quizState.begin(*this);
    currentState = this;
    Event startEvent(Event::HSM_START);
    dispatch(startEvent);
}

HandleResult_t MorseLittleProfessor::handle(Event& event) {
    switch (event.type) {
        case Event::HSM_START:
        case Event::ENTER:
            return HandleResult::transition(&showState);
        case Event::EXIT:
            // should not happen
            return HandleResult::handled();
        case Event::TICK:
            visualizer.renderState(event.data.tickData.time);
            return HandleResult::handled();
            break;
        case Event::BUTTONUP:
            break;
    }
    return HandleResult::parent();
}

// Method to look up the Morse pattern for the current letter
void MorseLittleProfessor::lookupMorsePattern(char letter) {
    if (letter < 'A' || letter > 'Z') {
        currentLetterPattern = nullptr;
        return;
    }

    // Calculate the index in the Morse table
    int index = letter - 'A' + 33; // Offset for ASCII table

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
}

void MorseLittleProfessor::startMorsePattern(uint32_t Time) {
    lastChangeTime = Time;
    currentPatternIndex = 0; // Reset the pattern index
    morsePixelOn = true; // Set the pixel to ON
    visualizer.setMorsePixel(true, currentPatternIndex); // Turn on the pixel
}


// Method to update the Morse pixel based on the current pattern and time
bool MorseLittleProfessor::updateMorsePixel(uint32_t newTime) {
    if (!currentLetterPattern) {
        // No pattern, turn off the pixel
        visualizer.setMorsePixel(false, currentPatternIndex); // Updated to pass symbolIndex
        return true;
    }

    if (morsePixelOn) {
        // If the pixel is currently ON, check if it's time to turn it OFF
        uint32_t onDuration = (currentLetterPattern[currentPatternIndex] == '-') ? DashTime : DotTime;
        if (newTime - lastChangeTime >= onDuration) {
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
                    visualizer.setMorsePixel(true, currentPatternIndex); // Turn on the pixel
                    morsePixelOn = true;
                    lastChangeTime = newTime;
                }
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

void MorseLittleProfessor::setOnlyLetter(char letter) {
    for (char i = 'A'; i < 'Z'+1; i++) {
        if (i == letter) {
            visualizer.setLetter(letter, true);
        } else {
            visualizer.setLetter(i, false);
        }
    }
}





void ShowState::begin(MorseLittleProfessor& pMorseLittleProfessor) {
    morseLittleProfessor = &pMorseLittleProfessor;
}
void QuizState::begin(MorseLittleProfessor& pMorseLittleProfessor) {
    morseLittleProfessor = &pMorseLittleProfessor;
}


HandleResult_t ShowState::handle(Event& event) {
    switch (event.type) {
        case Event::ENTER:
            currentLetter = 'A';
            // Notify the visualizer of the state change
            morseLittleProfessor->visualizer.setState(SHOW);
            morseLittleProfessor->lookupMorsePattern(currentLetter);

            morseLittleProfessor->visualizer.setMorsePattern(morseLittleProfessor->currentLetterPattern);
            morseLittleProfessor->startMorsePattern(event.data.tickData.time);
            morseLittleProfessor->visualizer.setLetter(currentLetter, true);
            return HandleResult::handled();
        case Event::EXIT:
            break;
        case Event::TICK:
            if (morseLittleProfessor->currentLetterPattern  && morseLittleProfessor->updateMorsePixel(event.data.tickData.time)) {
                // pattern done, repeat it
                morseLittleProfessor->lookupMorsePattern(currentLetter); // Get the Morse pattern for the current letter
                morseLittleProfessor->visualizer.setMorsePattern(morseLittleProfessor->currentLetterPattern);
                morseLittleProfessor->startMorsePattern(event.data.tickData.time);
            }
            morseLittleProfessor->visualizer.renderState(event.data.tickData.time);
            return HandleResult::handled();
        case Event::BUTTONDOWN:
            switch (event.data.buttonData.buttonId) {
                case BUTTON_SELECT_LETTER:
                    // Cycle through letters
                    currentLetter++;
                    if (currentLetter > 'Z') {
                        currentLetter = 'A';
                    }
                    morseLittleProfessor->lookupMorsePattern(currentLetter); // Update the Morse pattern for the new letter
                    morseLittleProfessor->visualizer.setMorsePattern(morseLittleProfessor->currentLetterPattern);
                    morseLittleProfessor->startMorsePattern(event.data.buttonData.time);
                    morseLittleProfessor->setOnlyLetter(currentLetter);
                    return HandleResult::handled();
                case BUTTON4:
                    return HandleResult::transition(&morseLittleProfessor->quizState);
                case BUTTON2:
                    break;
                case BUTTON3:
                    break;
            }
            break;
        case Event::BUTTONUP:
            break;
    }
    return HandleResult::parent();
}
#if 0
    // the Quiz feature
    // When beginning the quiz, we enter the quizsetup state, in which the user selects apattern of letters. By pressing BUTTON1,
    // the pattern can be changed. The user confirms by pressing BUTTON2. The quiz is then started.
    bool handleQuizSetup(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time) {
        switch (event) {
            case ENTER:
                // Notify the visualizer of the state change
                visualizer.setState(QUIZ);
                break;
            case EXIT:
                break;
            case TICK:
                visualizer.renderState(Time);
                return true;
                break;
            case BUTTONDOWN:
                switch (buttonId) {
                    case BUTTON_SELECT_LETTER:
                    case BUTTON_CONFIRM_SKIP :
                    case BUTTON_MODE_SELECT:
                        break;
                }
                break;
            case BUTTONUP:
                break;
        }
        return HandleResult::parent();
    }

    // The quiz is a simple game where the user has to key in the morse code of the letter shown on the display.
    // The user can press BUTTON_MORSE_INPUT to enter the morse code, BUTTON_SKIP to skip the letter and BUTTON_MODE_SELECT to exit the quiz.
    // When the user has entered the morse code, they should press BUTTON_CONFIRM. The program then checks if the pattern entered is correct.
    // If it is, the QuizCorrect state is entered, if it is not, the QuizIncorrect state is entered.
    // in the QuizCorrect state the visualizer shows a green light.
    // in the QuizIncorrect state, the visualizer shows a red light and shows the correct morse code pattern.

    bool handleQuizMain(EventType event, ButtonId buttonId, uint32_t buttonTime, uint32_t Time) {
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
                switch (buttonId) {
                    case BUTTON_SELECT_LETTER:
                    case BUTTON_SKIP:
                    case BUTTON_MODE_SELECT:
                        break;
                }
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

#endif
HandleResult_t QuizState::handle(Event& event) {
    switch (event.type) {
        case Event::ENTER:
            morseLittleProfessor->visualizer.setState(QUIZ);
            return HandleResult::handled();
        case Event::BUTTONDOWN:
            switch (event.data.buttonData.buttonId) {
                case BUTTON4:
                    return HandleResult::transition(&morseLittleProfessor->showState);
                break;
            }
            break;
    }
    return HandleResult::parent();
}

