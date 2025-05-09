// arduino ide builds all code in the folder, make it skip this Linux specific code
#ifndef ARDUINO
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <map>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "professor.h"


using namespace std;

StateVisualizer* visualizer;
MorseLittleProfessor* professor;

// Tracks the last time a character was received for each button
map<int, chrono::steady_clock::time_point> lastReceivedTime;

// Function to configure terminal for non-blocking input
void configureTerminal() {
    struct termios terminalSettings;

    // Get the current terminal settings
    tcgetattr(STDIN_FILENO, &terminalSettings);

    // Disable canonical mode and echo
    terminalSettings.c_lflag &= ~(ICANON | ECHO);

    // Set the new terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &terminalSettings);

    // Set stdin to non-blocking mode
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
}

// Function to simulate button presses using stdin
bool serialbutton(int button) {
    static map<char, int> buttonMapping = {
        {'A', 0}, // Button 0
        {'B', 1}, // Button 1
        {'C', 2}, // Button 2
        {'D', 3}  // Button 3
    };

    // Poll stdin for input
    char receivedChar;
    if (read(STDIN_FILENO, &receivedChar, 1) > 0) {
        if (receivedChar < 'z' && receivedChar > 'a') {
            receivedChar -= 32; // Convert lowercase to uppercase
        }
        // Check if the received character maps to a button
        if (buttonMapping.find(receivedChar) != buttonMapping.end()) {
            int mappedButton = buttonMapping[receivedChar];
            lastReceivedTime[mappedButton] = chrono::steady_clock::now();
        }
    }
#if 0
    // print the content of buttonMapping
cout << "lastreceivedtimes: ";
for (int i = 0; i < 4; ++i) {
    if (lastReceivedTime.find(i) != lastReceivedTime.end()) {
        cout << chrono::duration_cast<chrono::milliseconds>(lastReceivedTime[i].time_since_epoch()).count() << " ";
    } else {
        cout << "N/A ";
    }
}
cout << endl;
#endif
    // Check if the button argument matches and if the last received time is within 400ms
    auto now = chrono::steady_clock::now();
    if (lastReceivedTime.find(button) != lastReceivedTime.end() &&
        chrono::duration_cast<chrono::milliseconds>(now - lastReceivedTime[button]).count() <= 900) {
        return true; // Button is considered pressed
    }

    return false; // Button is not pressed
}

class SerialDisplay : public StateVisualizer {
private:
    bool letters[26];   // Array to store the enabled state of each letter (A-Z)
    StateIndicator currentState; // Stores the current state (enum)
    char currentLetter; // Stores the current letter
    bool morsePixelState; // Stores the current state of the Morse pixel (on or off)
    string morsePattern; // Stores the current Morse pattern

public:
    // Constructor
    SerialDisplay() : currentState(ROOT), currentLetter(' '), morsePixelState(false) {}

    // Override setState to update the current state
    void setState(StateIndicator state) override {
        currentState = state;
        cout << "State changed to: ";
        switch (currentState) {
            case ROOT:
                cout << "ROOT" << endl;
                break;
            case SHOW:
                cout << "SHOW" << endl;
                break;
            case RECOGNISE:
                cout << "RECOGNISE" << endl;
                break;
            case QUIZ:
                cout << "QUIZ" << endl;
                break;
            case MEMORIZE:
                cout << "MEMORIZE" << endl;
                break;
            case PLAYBACK:
                cout << "PLAYBACK" << endl;
                break;
        }
    }

    // Override renderState to render the current state
    void renderState(uint32_t Time) override {}

    // Override setLetter to update the current letter and its enabled state
    void setLetter(char letter, bool enabled) override {
        uint8_t letter_idx;
        if (letter >= 'a' && letter <= 'z') {
            letter_idx = (uint8_t)letter - 'a';
        } else if (letter >= 'A' && letter <= 'Z') {
            letter_idx = (uint8_t)letter - 'A';
        } else {
            cout << "Invalid letter. Must be A-Z or a-z." << endl;
            return;
        }
        if (letters[letter_idx] != enabled) {
            cout << "Letter " << letter << " is now " << (enabled ? "enabled" : "disabled") << endl;
            letters[letter_idx] = enabled;
        }
    }

    // Override setMorsePixel to control the Morse pixel (on or off)
    void setMorsePixel(bool on, int symbolIndex) override {
        if (morsePixelState != on) {
            cout << "Morse pixel is now " << (on ? "ON" : "OFF") << " for symbol index: " << symbolIndex << endl;
            morsePixelState = on;
        }
    }

    // Override setMorsePattern to set the Morse pattern
    void setMorsePattern(const char *pattern) override {
        morsePattern = pattern;
        if (pattern) {
        cout << "Morse pattern set to: " << morsePattern << endl;
        }
    }
};

// Update the main loop to use the Event tagged union structure
int main() {
    // Configure the terminal for non-blocking input
    configureTerminal();

    // Initialize the visualizer and professor
    visualizer = new SerialDisplay();
    professor = new MorseLittleProfessor(*visualizer);

    professor->begin();

    // Static arrays to store the previous state of the buttons and the time of the last BUTTONDOWN event
    static bool previousButtonStates[4] = {false, false, false, false};
    static chrono::steady_clock::time_point buttonDownTimes[4];

    while (true) {
        uint32_t now = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
        // Array to store the current state of the buttons
        bool currentButtonStates[4] = {
            serialbutton(0),
            serialbutton(1),
            serialbutton(2),
            serialbutton(3)
        };

        // Check for changes in button states
        for (int i = 0; i < 4; i++) {
            if (currentButtonStates[i] == previousButtonStates[i]) {
                continue;
            }
            uint32_t buttonTime = chrono::duration_cast<chrono::milliseconds>(
                chrono::steady_clock::now() - buttonDownTimes[i]
            ).count();

            // Create an Event based on the button state change
            Event event(currentButtonStates[i] ? Event::BUTTONDOWN : Event::BUTTONUP);
            event.data.buttonData = {
                static_cast<ButtonId>(i), 
                buttonTime,
                now
            };

            cout << "Button " << i << (event.type == Event::BUTTONDOWN ? " pressed" : " released") << endl;

            professor->dispatch(event);
            buttonDownTimes[i] = chrono::steady_clock::now();
        
        }

        // Create and handle a TICK event
        Event tickEvent(Event::TICK);
        tickEvent.data.tickData = {now};
        professor->dispatch(tickEvent);

        // Update the previous button states
        for (int i = 0; i < 4; i++) {
            previousButtonStates[i] = currentButtonStates[i];
        }

        // Delay for 20ms
        this_thread::sleep_for(chrono::milliseconds(20));

    }

    return 0;
}
#endif