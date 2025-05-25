#ifndef _ATTENUATOR_H_
#define _ATTENUATOR_H_

#include <stdint.h>

class Attenuator
{
private:
    uint8_t currentAttenuation;  // Current attenuation in dB (0-127)
    
public:
    Attenuator();
    ~Attenuator();
    
    // Set attenuation in dB (0-127)
    bool setAttenuation(uint8_t attenuation);
    
    // Get current attenuation in dB
    uint8_t getAttenuation() const;
    
    // Set attenuation using float value for compatibility
    bool setAttenuationFloat(float attenuation);
    
    // Get attenuation as float for compatibility
    float getAttenuationFloat() const;
    
    // Convert attenuation value to binary control word for MCP23008
    uint8_t getControlWord() const;
    
    // Apply the current attenuation to the hardware
    void updateHardware();
    
    // Initialize the attenuator
    void begin();
    
    // Increase attenuation by one step
    bool increaseAttenuation();
    
    // Decrease attenuation by one step
    bool decreaseAttenuation();
};

#endif /* _ATTENUATOR_H_ */
