#include <Arduino.h>
#include <Vrekrer_scpi_parser.h>
#include "scpi.h"


void setup() {
  Serial.begin(115200);

  // Register SCPI commands
  setupSCPI();
}

void loop() {
  scpi_attenuator.ProcessInput(Serial, "\n");
}

