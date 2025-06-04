#ifndef _SCPI_H_
#define _SCPI_H_

#define VREKRER_SCPI_PARSER_NO_IMPL

#include <Arduino.h>
#include <Vrekrer_scpi_parser.h>
#include "config.h"
#include "attenuator.h"

// Declare external SCPI parser object that will be defined in scpi.cpp
extern SCPI_Parser scpi_attenuator;

// Declare external attenuator object
extern Attenuator attenuator;

// Error queue for SCPI error handling
extern String errorQueue[10];
extern int errorQueueHead;
extern int errorQueueTail;
extern int errorQueueCount;

// Initializes and sets up the SCPI command interface
void setupSCPI();

// Add error to queue
void addError(int errorCode, const String& errorMessage);

// IEEE 488.2 Mandatory Commands
void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface);
void Reset(SCPI_C commands, SCPI_P parameters, Stream& interface);
void SelfTest(SCPI_C commands, SCPI_P parameters, Stream& interface);
void OperationComplete(SCPI_C commands, SCPI_P parameters, Stream& interface);
void OperationCompleteQuery(SCPI_C commands, SCPI_P parameters, Stream& interface);
void ClearStatus(SCPI_C commands, SCPI_P parameters, Stream& interface);

// System Commands
void SystemErrorQuery(SCPI_C commands, SCPI_P parameters, Stream& interface);
void SystemVersionQuery(SCPI_C commands, SCPI_P parameters, Stream& interface);

// Attenuation Commands
void setAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface);
void queryAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface);
void incrementAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface);
void decrementAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface);
void queryAttenuationMin(SCPI_C commands, SCPI_P parameters, Stream& interface);
void queryAttenuationMax(SCPI_C commands, SCPI_P parameters, Stream& interface);

// Network/WiFi Commands
void SystemNetworkStatus(SCPI_C commands, SCPI_P parameters, Stream& interface);
void SystemNetworkIP(SCPI_C commands, SCPI_P parameters, Stream& interface);
void SystemNetworkSSID(SCPI_C commands, SCPI_P parameters, Stream& interface);
void SystemNetworkInfo(SCPI_C commands, SCPI_P parameters, Stream& interface);
void SystemWiFiReset(SCPI_C commands, SCPI_P parameters, Stream& interface);
void SystemWiFiConfigure(SCPI_C commands, SCPI_P parameters, Stream& interface);

void myErrorHandler(SCPI_C commands, SCPI_P parameters, Stream& interface);

#endif // _SCPI_H_
