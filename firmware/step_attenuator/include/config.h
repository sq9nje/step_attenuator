#ifndef _CONFIG_H_
#define _CONFIG_H_

#define MIN_ATTENUATION 0.0f
#define MAX_ATTENUATION 127.0f
#define ATTENUATION_STEP 1.0f

#define SCL_PIN 32
#define SDA_PIN 33

#define ENC_A_PIN 39
#define ENC_B_PIN 36
#define ENC_SW_PIN 35

#define MCP23008_ADDRESS 0x27

// WiFi Configuration
#define WIFI_MANAGER_AP_NAME "StepAttenuator"
#define WIFI_MANAGER_AP_PASS "configure"
#define SCPI_TCP_PORT 5025
#define TELNET_PORT 23
#define HTTP_PORT 80
#define MAX_TCP_CLIENTS 4

#endif // _CONFIG_H_
