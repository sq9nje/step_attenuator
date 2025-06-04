#include "scpi.h"
#include "wifi_manager.h"

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

    // Register Network/WiFi Commands
    scpi_attenuator.RegisterCommand(F("SYSTem:NETwork:STATus?"), &SystemNetworkStatus);
    scpi_attenuator.RegisterCommand(F("SYSTem:NETwork:IP?"), &SystemNetworkIP);
    scpi_attenuator.RegisterCommand(F("SYSTem:NETwork:SSID?"), &SystemNetworkSSID);
    scpi_attenuator.RegisterCommand(F("SYSTem:NETwork:INFO?"), &SystemNetworkInfo);
    scpi_attenuator.RegisterCommand(F("SYSTem:WIFI:RESet"), &SystemWiFiReset);
    scpi_attenuator.RegisterCommand(F("SYSTem:WIFI:CONFigure"), &SystemWiFiConfigure);

    scpi_attenuator.SetErrorHandler(&myErrorHandler);
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

    switch(scpi_attenuator.last_error){
    case SCPI_Parser::ErrorCode::BufferOverflow: 
      interface.println(F("Buffer overflow error"));
      break;
    case SCPI_Parser::ErrorCode::Timeout:
      interface.println(F("Communication timeout error"));
      break;
    case SCPI_Parser::ErrorCode::UnknownCommand:
      interface.println(F("Unknown command received"));
      break;
    case SCPI_Parser::ErrorCode::NoError:
      interface.println(F("No Error"));
      break;
  }
  scpi_attenuator.last_error = SCPI_Parser::ErrorCode::NoError;
}

void myErrorHandler(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  //This function is called every time an error occurs

  /* The error type is stored in my_instrument.last_error
     Possible errors are:
       SCPI_Parser::ErrorCode::NoError
       SCPI_Parser::ErrorCode::UnknownCommand
       SCPI_Parser::ErrorCode::Timeout
       SCPI_Parser::ErrorCode::BufferOverflow
  */

  /* For BufferOverflow errors, the rest of the message, still in the interface
  buffer or not yet received, will be processed later and probably 
  trigger another kind of error.
  Here we flush the incomming message*/
  if (scpi_attenuator.last_error == SCPI_Parser::ErrorCode::BufferOverflow) {
    delay(2);
    while (interface.available()) {
      delay(2);
      interface.read();
    }
  }

  /*
  For UnknownCommand errors, you can get the received unknown command and
  parameters from the commands and parameters variables.
  */
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

// Network/WiFi Commands Implementation
void SystemNetworkStatus(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    WiFiStatus status = getWiFiStatus();
    switch (status) {
        case WIFI_CONNECTED:
            interface.println(F("CONNECTED"));
            break;
        case WIFI_CONNECTING:
            interface.println(F("CONNECTING"));
            break;
        case WIFI_DISCONNECTED:
            interface.println(F("DISCONNECTED"));
            break;
        case WIFI_AP_MODE:
            interface.println(F("ACCESS_POINT"));
            break;
        case WIFI_ERROR:
            interface.println(F("ERROR"));
            break;
        default:
            interface.println(F("UNKNOWN"));
            break;
    }
}

void SystemNetworkIP(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    if (getWiFiStatus() == WIFI_CONNECTED && deviceIP.length() > 0) {
        interface.println(deviceIP);
    } else {
        interface.println(F("0.0.0.0"));
    }
}

void SystemNetworkSSID(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    if (getWiFiStatus() == WIFI_CONNECTED && deviceSSID.length() > 0) {
        interface.println(deviceSSID);
    } else {
        interface.println(F("Not Connected"));
    }
}

void SystemNetworkInfo(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    String info = getConnectionInfo();
    info.replace("\n", ";");  // Replace newlines with semicolons for SCPI response
    interface.println(info);
}

void SystemWiFiReset(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    operationComplete = false;
    interface.println(F("WiFi settings will be reset. Device will restart."));
    delay(100);  // Give time for response to be sent
    resetWiFiSettings();
    operationComplete = true;
}

void SystemWiFiConfigure(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    if (parameters.Size() >= 2) {
        // Parameters provided: SSID and password
        String ssid = String(parameters[0]);
        String password = String(parameters[1]);
        
        if (ssid.length() > 0) {
            operationComplete = false;
            interface.println(F("Configuring WiFi..."));
            
            // Disconnect current WiFi
            WiFi.disconnect();
            delay(1000);
            
            // Try to connect with new credentials
            WiFi.begin(ssid.c_str(), password.c_str());
            
            // Wait for connection (up to 30 seconds)
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 60) {
                delay(500);
                attempts++;
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                deviceIP = WiFi.localIP().toString();
                deviceSSID = WiFi.SSID();
                currentWiFiStatus = WIFI_CONNECTED;
                interface.println(F("WiFi configured successfully"));
                interface.print(F("IP: "));
                interface.println(deviceIP);
            } else {
                currentWiFiStatus = WIFI_DISCONNECTED;
                interface.println(F("WiFi configuration failed"));
                addError(-350, "Queue overflow;WiFi connection failed");
            }
            operationComplete = true;
        } else {
            addError(-109, "Missing parameter;SSID cannot be empty");
            interface.println(F("-109,\"Missing parameter;SSID cannot be empty\""));
        }
    } else {
        // No parameters - start configuration portal
        operationComplete = false;
        interface.println(F("Starting WiFi configuration portal..."));
        interface.print(F("Connect to AP: "));
        interface.println(F(WIFI_MANAGER_AP_NAME));
        interface.print(F("Password: "));
        interface.println(F(WIFI_MANAGER_AP_PASS));
        interface.println(F("Navigate to 192.168.4.1 to configure"));
        
        // Reset WiFi settings to force configuration portal
        resetWiFiSettings();
        operationComplete = true;
    }
}
