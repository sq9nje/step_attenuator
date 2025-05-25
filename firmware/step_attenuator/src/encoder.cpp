#include "encoder.h"

// Global encoder state
EncoderState encoder = {0};

void initEncoder() {
    // Initialize encoder pins
    pinMode(ENC_A_PIN, INPUT_PULLUP);
    pinMode(ENC_B_PIN, INPUT_PULLUP);
    pinMode(ENC_SW_PIN, INPUT_PULLUP);
    
    // Initialize encoder state
    encoder.lastEncoded = 0;
    encoder.encoderValue = 0;
    encoder.aFlag = false;
    encoder.bFlag = false;
    encoder.reading = false;
    encoder.buttonPressed = false;
    encoder.buttonReleased = false;
    encoder.lastButtonTime = 0;
    encoder.lastEncoderTime = 0;
    encoder.buttonState = false;
    encoder.lastButtonState = false;
    
    // Attach interrupts
    attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_B_PIN), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_SW_PIN), buttonISR, CHANGE);
    
    Serial.println("Rotary encoder initialized");
}

void IRAM_ATTR encoderISR() {
    // Debouncing
    if (millis() - encoder.lastEncoderTime < ENCODER_DEBOUNCE_TIME) {
        return;
    }
    encoder.lastEncoderTime = millis();
    
    // Read encoder pins
    bool aState = digitalRead(ENC_A_PIN);
    bool bState = digitalRead(ENC_B_PIN);
    
    // Encode the current state
    int8_t encoded = (aState << 1) | bState;
    int8_t sum = (encoder.lastEncoded << 2) | encoded;
    
    // Determine direction based on state transitions
    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
        encoder.encoderValue++;
    } else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
        encoder.encoderValue--;
    }
    
    encoder.lastEncoded = encoded;
}

void IRAM_ATTR buttonISR() {
    // Debouncing
    if (millis() - encoder.lastButtonTime < BUTTON_DEBOUNCE_TIME) {
        return;
    }
    encoder.lastButtonTime = millis();
    
    bool currentState = !digitalRead(ENC_SW_PIN); // Active low
    
    if (currentState && !encoder.buttonState) {
        encoder.buttonPressed = true;
    } else if (!currentState && encoder.buttonState) {
        encoder.buttonReleased = true;
    }
    
    encoder.buttonState = currentState;
}

void handleEncoder() {
    static int32_t lastEncoderValue = 0;
    static bool buttonProcessed = false;
    static unsigned long buttonPressStart = 0;
    
    // Handle encoder rotation
    if (encoder.encoderValue != lastEncoderValue) {
        int32_t delta = encoder.encoderValue - lastEncoderValue;
        
        // Apply sensitivity - every 4 encoder steps = 1 attenuation step
        if (abs(delta) >= 4) {
            if (delta > 0) {
                // Clockwise - increase attenuation
                for (int i = 0; i < abs(delta) / 4; i++) {
                    if (!attenuator.increaseAttenuation()) {
                        break; // Stop if we hit the maximum
                    }
                }
                Serial.println("Encoder: Increased attenuation to " + String(attenuator.getAttenuation()) + " dB");
            } else {
                // Counter-clockwise - decrease attenuation
                for (int i = 0; i < abs(delta) / 4; i++) {
                    if (!attenuator.decreaseAttenuation()) {
                        break; // Stop if we hit the minimum
                    }
                }
                Serial.println("Encoder: Decreased attenuation to " + String(attenuator.getAttenuation()) + " dB");
            }
            
            // Reset encoder value to prevent accumulation
            lastEncoderValue = encoder.encoderValue;
        }
    }
    
    // Handle button press
    if (encoder.buttonPressed && !buttonProcessed) {
        buttonPressStart = millis();
        buttonProcessed = true;
        encoder.buttonPressed = false;
    }
    
    // Handle button release
    if (encoder.buttonReleased && buttonProcessed) {
        unsigned long pressDuration = millis() - buttonPressStart;
        
        if (pressDuration >= BUTTON_LONG_PRESS_TIME) {
            // Long press - reset to 0 dB
            attenuator.setAttenuation(0);
            Serial.println("Encoder: Long press - reset to 0 dB");
        } else {
            // Short press - toggle between preset values or reset
            uint8_t currentAtt = attenuator.getAttenuation();
            if (currentAtt == 0) {
                attenuator.setAttenuation(30); // Go to 30 dB
                Serial.println("Encoder: Short press - set to 30 dB");
            } else if (currentAtt == 30) {
                attenuator.setAttenuation(60); // Go to 60 dB  
                Serial.println("Encoder: Short press - set to 60 dB");
            } else {
                attenuator.setAttenuation(0); // Reset to 0 dB
                Serial.println("Encoder: Short press - reset to 0 dB");
            }
        }
        
        buttonProcessed = false;
        encoder.buttonReleased = false;
    }
    
    // Reset button processing if button is stuck
    if (buttonProcessed && (millis() - buttonPressStart > 5000)) {
        buttonProcessed = false;
    }
}

int8_t getEncoderDirection() {
    static int32_t lastValue = 0;
    int32_t currentValue = encoder.encoderValue;
    
    if (currentValue > lastValue) {
        lastValue = currentValue;
        return 1; // Clockwise
    } else if (currentValue < lastValue) {
        lastValue = currentValue;
        return -1; // Counter-clockwise
    }
    
    return 0; // No change
}

bool isButtonPressed() {
    if (encoder.buttonPressed) {
        encoder.buttonPressed = false;
        return true;
    }
    return false;
}

bool isButtonReleased() {
    if (encoder.buttonReleased) {
        encoder.buttonReleased = false;
        return true;
    }
    return false;
}

void resetEncoderValue() {
    encoder.encoderValue = 0;
}
