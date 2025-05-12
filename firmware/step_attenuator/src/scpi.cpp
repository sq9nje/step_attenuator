#include "scpi.h"

SCPI_Parser scpi_attenuator;

// Global variable to store current attenuation value
static float current_attenuation = 0.0;

void setupSCPI() {
    // Register the standard *IDN? command
    scpi_attenuator.RegisterCommand(F("*IDN?"), &Identify);
    
    // Register attenuation commands
    scpi_attenuator.RegisterCommand(F("ATTenuator:VALue"), &setAttenuation);
    scpi_attenuator.RegisterCommand(F("ATTenuator:VALue?"), &queryAttenuation);
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    interface.println(F("MyCompany,Digital-Attenuator,SN001,v1.0"));
}

void setAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    // Check if we have a parameter
    if (parameters.Size() > 0) {
        float new_attenuation = String(parameters[0]).toFloat();
        
        // Add bounds checking (adjust these values according to your hardware)
        if (new_attenuation >= 0.0 && new_attenuation <= 31.5) {
            current_attenuation = new_attenuation;
            // TODO: Add your hardware control code here
            // For example: setAttenuatorValue(current_attenuation);
        } else {
            interface.println(F("Error: Attenuation value out of range (0-31.5 dB)"));
        }
    } else {
        interface.println(F("Error: Value missing"));
    }
}

void queryAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    interface.println(current_attenuation);
}