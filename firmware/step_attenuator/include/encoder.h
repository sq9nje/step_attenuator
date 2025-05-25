#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <Arduino.h>
#include "config.h"
#include "attenuator.h"

// External attenuator object
extern Attenuator attenuator;

// Encoder state structure
struct EncoderState {
    int8_t lastEncoded;
    int32_t encoderValue;
    volatile bool aFlag;
    volatile bool bFlag;
    volatile bool reading;
    volatile bool buttonPressed;
    volatile bool buttonReleased;
    unsigned long lastButtonTime;
    unsigned long lastEncoderTime;
    bool buttonState;
    bool lastButtonState;
};

// External encoder state
extern EncoderState encoder;

// Function declarations
void initEncoder();
void handleEncoder();
void IRAM_ATTR encoderISR();
void IRAM_ATTR buttonISR();
int8_t getEncoderDirection();
bool isButtonPressed();
bool isButtonReleased();
void resetEncoderValue();

// Encoder sensitivity settings
#define ENCODER_DEBOUNCE_TIME 2    // milliseconds
#define BUTTON_DEBOUNCE_TIME 50    // milliseconds
#define BUTTON_LONG_PRESS_TIME 1000 // milliseconds for long press

#endif // _ENCODER_H_
