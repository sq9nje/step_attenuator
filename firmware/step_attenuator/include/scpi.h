#ifndef _SCPI_H_
#define _SCPI_H_

#define VREKRER_SCPI_PARSER_NO_IMPL

#include <Arduino.h>
#include <Vrekrer_scpi_parser.h>

// Declare external SCPI parser object that will be defined in scpi.cpp
extern SCPI_Parser scpi_attenuator;

// Initializes and sets up the SCPI command interface
void setupSCPI();

// Responds to the standard SCPI *IDN? query with device identification information
void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface);

// Sets the attenuation level of the device based on input parameters
void setAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface);

// Returns the current attenuation level of the device
void queryAttenuation(SCPI_C commands, SCPI_P parameters, Stream& interface);


#endif // _SCPI_H_

