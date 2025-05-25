#include "scpi.h"

SCPI_Parser scpi_attenuator;

// Error queue implementation
String errorQueue[10];
int errorQueueHead = 0;
int errorQueueTail = 0;
int errorQueueCount = 0;
bool operationComplete = true;

void setupSCPI() {
    // Set a longer timeout for terminals that send each character separately
    scpi_attenuator.timeout = 60000;
    
    // Register IEEE 488.2 Mandatory Commands
    scpi_attenuator.RegisterCommand(F("*IDN?"), &Identify);
    scpi_attenuator.RegisterCommand(F("*RST"), &Reset);
    scpi_attenuator.RegisterCommand(F("*TST?"), &SelfTest);
    scpi_attenuator.RegisterCommand(F("*OPC"), &OperationComplete);
    scpi_attenuator.RegisterCommand(F("*OPC?"), &OperationCompleteQuery);
    scpi_attenuator.RegisterCommand(F("*CLS"), &ClearStatus);
    
    // Register System Commands
    scpi_attenuator.RegisterCommand(F("SYSTem:ERRor?"), &SystemErrorQuery);
    scpi_attenuator.RegisterCommand(F("SYSTem:VERSion?"), &SystemVersionQuery);
    
    // Register attenuation commands with standard hierarchy
    scpi_attenuator.RegisterCommand(F("ATTenuator:VALue"), &setAttenuation);
    scpi_attenuator.RegisterCommand(F("ATTenuator:VALue?"), &queryAttenuation);
    scpi_attenuator.RegisterCommand(F("ATTenuator:INCrement"), &incrementAttenuation);
    scpi_attenuator.RegisterCommand(F("ATTenuator:DECrement"), &decrementAttenuation);
    scpi_attenuator.RegisterCommand(F("ATTenuator:VALue:MINimum?"), &queryAttenuationMin);
    scpi_attenuator.RegisterCommand(F("ATTenuator:VALue:MAXimum?"), &queryAttenuationMax);

}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    interface.println(F("SQ9NJE,Digital-Attenuator,0-127dB,v1.0"));
}

void setAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    // Check if we have a parameter
    if (parameters.Size() > 0) {
        float new_attenuation = String(parameters[0]).toFloat();
        
        // Use the attenuator class to set the value (includes bounds checking)
        if (attenuator.setAttenuationFloat(new_attenuation)) {
            // Successfully set the attenuation
        } else {
            addError(-224, "Illegal parameter value;Attenuation out of range 0-127 dB");
            interface.println(F("-224,\"Illegal parameter value;Attenuation out of range 0-127 dB\""));
        }
    } else {
        addError(-109, "Missing parameter");
        interface.println(F("-109,\"Missing parameter\""));
    }
}

void queryAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    interface.println(attenuator.getAttenuation());
}

void incrementAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    if (attenuator.increaseAttenuation()) {
        // Successfully increased attenuation
    } else {
        addError(-224, "Illegal parameter value;Attenuation already at maximum");
        interface.println(F("-224,\"Illegal parameter value;Attenuation already at maximum\""));
    }
}

void decrementAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    if (attenuator.decreaseAttenuation()) {
        // Successfully decreased attenuation
    } else {
        addError(-224, "Illegal parameter value");
        interface.println(F("-224,\"Illegal parameter value;Attenuation already at minimum\""));
    }
}

// Error queue management
void addError(int errorCode, const String& errorMessage) {
    if (errorQueueCount < 10) {
        errorQueue[errorQueueTail] = String(errorCode) + ",\"" + errorMessage + "\"";
        errorQueueTail = (errorQueueTail + 1) % 10;
        errorQueueCount++;
    }
}

// IEEE 488.2 Mandatory Commands
void Reset(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    operationComplete = false;
    attenuator.setAttenuation(0);  // Reset to 0 dB
    // Clear error queue
    errorQueueHead = 0;
    errorQueueTail = 0;
    errorQueueCount = 0;
    operationComplete = true;
}

void SelfTest(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    // Basic self-test - verify attenuator communication
    operationComplete = false;
    uint8_t currentAtt = attenuator.getAttenuation();
    attenuator.setAttenuation(0);
    attenuator.setAttenuation(currentAtt);  // Restore original value
    operationComplete = true;
    interface.println(F("0"));  // 0 = passed, non-zero = failed
}

void OperationComplete(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    operationComplete = true;
}

void OperationCompleteQuery(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    interface.println(operationComplete ? F("1") : F("0"));
}

void ClearStatus(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    // Clear error queue
    errorQueueHead = 0;
    errorQueueTail = 0;
    errorQueueCount = 0;
    operationComplete = true;
}

// System Commands
void SystemErrorQuery(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    if (errorQueueCount > 0) {
        interface.println(errorQueue[errorQueueHead]);
        errorQueueHead = (errorQueueHead + 1) % 10;
        errorQueueCount--;
    } else {
        interface.println(F("0,\"No error\""));
    }
}

void SystemVersionQuery(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    interface.println(F("1999.0"));  // SCPI version 1999.0
}

// Enhanced attenuation commands
void queryAttenuationMin(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    interface.println(MIN_ATTENUATION);  // Minimum attenuation in dB
}

void queryAttenuationMax(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    interface.println(MAX_ATTENUATION);  // Maximum attenuation in dB
}
