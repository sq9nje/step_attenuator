#ifndef _WIFI_MANAGER_H_
#define _WIFI_MANAGER_H_

#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Arduino.h>
#include "config.h"

// WiFi status enumeration
enum WiFiStatus {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_AP_MODE,
    WIFI_ERROR
};

// Structure to hold client information
struct TCPClient {
    WiFiClient client;
    bool connected;
    unsigned long lastActivity;
};

// External variables
extern WiFiServer scpiServer;
extern WiFiServer telnetServer;
extern WebServer webServer;
extern TCPClient tcpClients[MAX_TCP_CLIENTS];
extern WiFiStatus currentWiFiStatus;
extern String deviceIP;
extern String deviceSSID;

// Function declarations
void initWiFi();
void handleWiFi();
void startTCPServers();
void setupWebServer();
void handleTCPClients();
void handleWebServer();
void disconnectClient(int clientIndex);
WiFiStatus getWiFiStatus();
String getConnectionInfo();
void resetWiFiSettings();

// Web interface functions
void handleRoot();
void handleStatus();
void handleControl();
void handleReset();
void handleStaticFileOrNotFound();

// Helper functions
String getMimeType(const String& filename);

#endif // _WIFI_MANAGER_H_
