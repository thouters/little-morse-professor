#include "professor.h"
#include "morsetable.h"
#include <string.h>

#ifndef ARDUINO
// long live printf debugging ^_^
#include <termios.h>
#include <unistd.h>
#include <iostream>
using std::cout;
using std::endl;
#endif

void MorseLittleProfessor::begin() {
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
const char * MorseLittleProfessor::lookupMorsePattern(char letter) {
    if (letter < 'A' || letter > 'Z') {
        return nullptr;
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
    return buffer; // skip the character's ascii value at index 0
#else
    return (const char*)morsecode_table[index];
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





HandleResult_t ShowState::handle(Event& event) {
    switch (event.type) {
        case Event::ENTER:
            currentLetter = 'A';
            // Notify the visualizer of the state change
            morseLittleProfessor->visualizer.setState(SHOW);
            morseLittleProfessor->currentLetterPattern = morseLittleProfessor->lookupMorsePattern(currentLetter);

            morseLittleProfessor->visualizer.setMorsePattern(morseLittleProfessor->currentLetterPattern);
            morseLittleProfessor->startMorsePattern(event.data.tickData.time);
            morseLittleProfessor->visualizer.setLetter(currentLetter, true);
            return HandleResult::handled();
        case Event::EXIT:
            break;
        case Event::TICK:
            if (morseLittleProfessor->currentLetterPattern  && morseLittleProfessor->updateMorsePixel(event.data.tickData.time)) {
                // pattern done, repeat it
                morseLittleProfessor->currentLetterPattern  = morseLittleProfessor->lookupMorsePattern(currentLetter); // Get the Morse pattern for the current letter
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
                    morseLittleProfessor->currentLetterPattern = morseLittleProfessor->lookupMorsePattern(currentLetter); // Update the Morse pattern for the new letter
                    morseLittleProfessor->visualizer.setMorsePattern(morseLittleProfessor->currentLetterPattern);
                    morseLittleProfessor->startMorsePattern(event.data.buttonData.time);
                    morseLittleProfessor->setOnlyLetter(currentLetter);
                    return HandleResult::handled();
                case BUTTON_MODE_SELECT:
                    return HandleResult::transition(&morseLittleProfessor->recogniseState);
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
HandleResult_t QuizSetup::handle(Event& event) {
}

    // The quiz is a simple game where the user has to key in the morse code of the letter shown on the display.
    // The user can press BUTTON_MORSE_INPUT to enter the morse code, BUTTON_SKIP to skip the letter and BUTTON_MODE_SELECT to exit the quiz.
    // When the user has entered the morse code, they should press BUTTON_CONFIRM. The program then checks if the pattern entered is correct.
    // If it is, the QuizCorrect state is entered, if it is not, the QuizIncorrect state is entered.
    // in the QuizCorrect state the visualizer shows a green light.
    // in the QuizIncorrect state, the visualizer shows a red light and shows the correct morse code pattern.

HandleResult_t QuizMain::handle(Event& event) {
}

HandleResult_t Memorize::handle(Event& event) {
}

HandleResult_t Record::handle(Event& event) {
}
HandleResult_t Playback::handle(Event& event) {
}
HandleResult_t Recognise::handle(Event& event) {
}
#endif

HandleResult_t RecogniseState::handle(Event& event) {
    switch (event.type) {
        case Event::ENTER:
            morseLittleProfessor->visualizer.setState(RECOGNISE);
            lastButtonPressTime = event.data.tickData.time;
            markCounter = 0;
            cursorState = false;
            nextCursorUpdate = event.data.tickData.time;

            memset(markTimes, 0, sizeof(markTimes));
            memset(spaceTimes, 0, sizeof(spaceTimes));
            memset(morsePattern, 0, sizeof(morsePattern)); 
            morseLittleProfessor->visualizer.setMorsePattern(morsePattern); 
            morseLittleProfessor->setOnlyLetter(' '); // Clear the letter display   
            morseLittleProfessor->visualizer.renderState(event.data.tickData.time);
            return HandleResult::handled();
        case Event::TICK:
            if (event.data.tickData.time > nextCursorUpdate) {
                cursorState = cursorState ^ true;
                morseLittleProfessor->visualizer.setMorsePixel(cursorState, markCounter);
                nextCursorUpdate = event.data.tickData.time + 500;
            }
            morseLittleProfessor->visualizer.renderState(event.data.tickData.time);
            if (event.data.tickData.time - lastButtonPressTime > 3000) {
                // Timeout, reset
                return HandleResult::transition(this);
            }
            break;
        case Event::BUTTONDOWN:
            switch (event.data.buttonData.buttonId) {
                case BUTTON_SELECT_LETTER:
                    // reset
                    return HandleResult::transition(this);
                    break;
                case BUTTON_MORSE_INPUT:  {
                    // Record the pulse
                    uint32_t uptime = event.data.buttonData.time - lastButtonPressTime;
                    spaceTimes[markCounter] = uptime;
                    lastButtonPressTime = event.data.buttonData.time;
                    evaluateInput();
                    morseLittleProfessor->visualizer.renderState(event.data.buttonData.time);
                    return HandleResult::handled();
                    break;
                }
                case BUTTON_MODE_SELECT:
                    return HandleResult::transition(&morseLittleProfessor->showState);
                    break;
            }
            break;
        case Event::BUTTONUP:
            switch (event.data.buttonData.buttonId) {
                case BUTTON_MORSE_INPUT: 
                    // Record the pulse
                    uint32_t downtime = event.data.buttonData.time - lastButtonPressTime;
                    markTimes[markCounter] = downtime;
                    morseLittleProfessor->visualizer.setMorsePixel(false, markCounter);
                    markCounter++;
                    lastButtonPressTime = event.data.buttonData.time;
                    evaluateInput();
                    cursorState = false;
                    nextCursorUpdate = event.data.buttonData.time;
                    morseLittleProfessor->visualizer.renderState(event.data.buttonData.time);
                    return HandleResult::handled();
                    break;
            }
            break;
    }
    return HandleResult::parent();
}

// go over the recorded pulses and check if they match the morse code table
void RecogniseState::evaluateInput(void)
{
    // calculate the average of the spaceTimes
    uint32_t spaceAverage = 0;
    if (markCounter >= 1) {
        uint32_t spaceAccumulator = 0;
        for (int i = 1;  // skip the leading space
                i < markCounter; i++) {
            spaceAccumulator += spaceTimes[i];
        }
        spaceAverage = spaceAccumulator / markCounter;
    } else {
        // single mark pattern - use a default
        spaceAverage = 900; //ms
    }

    // first bubble sort the length of the markTimes
    uint32_t sortedMarks[32];
    memcpy(sortedMarks, markTimes, sizeof(sortedMarks));

    for (int i = 0; i < markCounter; i++) {
        for (int j = 0; j < markCounter - 1; j++) {
            if (sortedMarks[j] > sortedMarks[j + 1]) {
                uint32_t temp = sortedMarks[j];
                sortedMarks[j] = sortedMarks[j + 1];
                sortedMarks[j + 1] = temp;
            }
        }
    }

    // Converte the pulse lengths to dit and dah

    // recognise markTime of the dit pulses which are 1/3rd of the length of a dah:

    #define APROXIMATELY(expected,actual,margin) (\
        (expected - margin < actual) && (expected + margin > actual) \
    )

    uint32_t dahAverage = 0;
    uint32_t dahAccumulator = 0;
    uint32_t ditAverage = spaceAverage;
    uint32_t ditAccumulator = 0;
    uint32_t nbrOfDahs = 0;
    int i;
    for (i=0; i < markCounter; i++) {
        if (APROXIMATELY(spaceAverage,sortedMarks[i],100)) {
            break;
        }
        dahAccumulator += sortedMarks[i];
        dahAverage = dahAccumulator / (i + 1);
        nbrOfDahs++;

        if ((i + 1) < markCounter) {
            if (APROXIMATELY(dahAverage/3,sortedMarks[i+1],200)) {
                break;
            }
        }
    }

    // if we hit an entry that is aprox the spaceTime, we can assume that we are at the boundary between dahs and dits
    for (;  i < markCounter; i++) {
        // calculate the average of the remaining marks as ditAverage.
        ditAverage += sortedMarks[i];
        ditAverage = ditAccumulator / (i-nbrOfDahs + 1);
    } 

    // convert the marks to dit and dahs
    for (int i = 0; i < markCounter; i++) {
        if (APROXIMATELY(ditAverage,markTimes[i],100)) {
            // this is a dit
            morsePattern[i] = '.';
        } else {
            morsePattern[i] = '-';
        }
    }
    morsePattern[markCounter] = '\0';

    // now we have the morse pattern, we can check if it is in the morse code table
    for (char letter= 'A'; letter < 'Z'+1; letter++) {
        const char *letterPattern = morseLittleProfessor->lookupMorsePattern(letter);
        if (strcmp(morsePattern, letterPattern) == 0) {
            morseLittleProfessor->visualizer.setLetter(letter, true);
        } else {
            morseLittleProfessor->visualizer.setLetter(letter, false);
        }
    }
}