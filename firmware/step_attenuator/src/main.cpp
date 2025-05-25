#include <Arduino.h>
#include <Vrekrer_scpi_parser.h>
#include <Wire.h>

#include "config.h"
#include "scpi.h"
#include "display.h"
#include "attenuator.h"
#include "wifi_manager.h"
#include "encoder.h"

// Global attenuator instance
Attenuator attenuator;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize attenuator
  attenuator.begin();

  // Initialize rotary encoder
  initEncoder();
  
  // Initialize display and show splash screen
  display_init();
  display_splash();

  // Register SCPI commands
  setupSCPI();

  Serial.println("Programmable Attenuator");
  
  // Initialize WiFi and networking
  initWiFi();
  
  // Keep splash screen visible for a moment
  delay(2000);
  
  Serial.println("System ready");
  Serial.println("Control interfaces available:");
  Serial.println("- Rotary encoder (manual control)");
  Serial.println("- Serial port (USB SCPI)");
  if (getWiFiStatus() == WIFI_CONNECTED) {
    Serial.println("- TCP port " + String(SCPI_TCP_PORT) + " (SCPI)");
    Serial.println("- TCP port " + String(TELNET_PORT) + " (Telnet)");
    Serial.println("- HTTP port " + String(HTTP_PORT) + " (Web interface)");
  }
  Serial.println("");
  Serial.println("Encoder controls:");
  Serial.println("- Turn: Adjust attenuation (±1 dB per 4 clicks)");
  Serial.println("- Short press: Cycle presets (0→30→60→0 dB)");
  Serial.println("- Long press (1s): Reset to 0 dB");
}

void loop() {
  // Handle serial SCPI commands
  scpi_attenuator.ProcessInput(Serial, "\r\n");
  
  // Handle rotary encoder input
  handleEncoder();
  
  // Handle WiFi and TCP connections
  handleWiFi();
  
  // Update display with current status
  String statusLine = "";
  WiFiStatus wifiStatus = getWiFiStatus();
  switch (wifiStatus) {
    case WIFI_CONNECTED:
      statusLine = "IP: " + deviceIP;
      break;
    case WIFI_CONNECTING:
      statusLine = "WiFi: Connecting...";
      break;
    case WIFI_DISCONNECTED:
      statusLine = "WiFi: Disconnected";
      break;
    case WIFI_AP_MODE:
      statusLine = "WiFi: AP Mode";
      break;
    case WIFI_ERROR:
      statusLine = "WiFi: Error";
      break;
  }
  
  display_update(attenuator.getAttenuation(), statusLine.c_str());

}
