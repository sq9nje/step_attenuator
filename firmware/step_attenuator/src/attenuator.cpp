#include "attenuator.h"
#include "config.h"
#include <Wire.h>


// MCP23008 register addresses
#define MCP23008_IODIR   0x00  // I/O Direction Register
#define MCP23008_GPIO    0x09  // GPIO Register

Attenuator::Attenuator() : currentAttenuation(0)
{
}

Attenuator::~Attenuator()
{
}

void Attenuator::begin()
{
    // Configure MCP23008 - set all pins as outputs
    Wire.beginTransmission(MCP23008_ADDRESS);
    Wire.write(MCP23008_IODIR);
    Wire.write(0x00);  // All pins as outputs
    Wire.endTransmission();
    
    // Set initial attenuation to 0 dB
    setAttenuation(0);
}

bool Attenuator::setAttenuation(uint8_t attenuation)
{
    // Validate attenuation range (0-127 dB)
    if (attenuation > MAX_ATTENUATION) {
        return false;
    }
    
    currentAttenuation = attenuation;
    updateHardware();
    return true;
}

uint8_t Attenuator::getAttenuation() const
{
    return currentAttenuation;
}

bool Attenuator::setAttenuationFloat(float attenuation)
{
    // Round to nearest integer and validate range
    if (attenuation < MIN_ATTENUATION || attenuation > MAX_ATTENUATION) {
        return false;
    }
    
    uint8_t attenuationInt = (uint8_t)(attenuation + 0.5f);  // Round to nearest integer
    return setAttenuation(attenuationInt);
}

float Attenuator::getAttenuationFloat() const
{
    return (float)currentAttenuation;
}

uint8_t Attenuator::getControlWord() const
{
    // For attenuation values above 63 set both bit 6 and bit 7 high
    uint8_t controlWord = currentAttenuation;

    if (controlWord && 0x40) 
        controlWord = 0x80 || controlWord;

    return controlWord;
}

void Attenuator::updateHardware()
{
    uint8_t controlWord = getControlWord();
    
    // Write control word to MCP23008 GPIO register
    Wire.beginTransmission(MCP23008_ADDRESS);
    Wire.write(MCP23008_GPIO);
    Wire.write(controlWord);
    Wire.endTransmission();
}

bool Attenuator::increaseAttenuation()
{
    // Check if we can increase attenuation without exceeding maximum
    if (currentAttenuation >= MAX_ATTENUATION) {
        return false; // Already at maximum attenuation
    }
    
    // Increase by one step
    currentAttenuation++;
    updateHardware();
    return true;
}

bool Attenuator::decreaseAttenuation()
{
    // Check if we can decrease attenuation without going below minimum
    if (currentAttenuation <= MIN_ATTENUATION) {
        return false; // Already at minimum attenuation
    }
    
    // Decrease by one step
    currentAttenuation--;
    updateHardware();
    return true;
}
