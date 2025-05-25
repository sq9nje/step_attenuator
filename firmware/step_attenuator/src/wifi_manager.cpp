#include "wifi_manager.h"
#include "config.h"
#include "attenuator.h"
#include "scpi.h"
#include "SPIFFS.h"

// Global WiFi objects
WiFiManager wifiManager;
WiFiServer scpiServer(SCPI_TCP_PORT);
WiFiServer telnetServer(TELNET_PORT);
WebServer webServer(HTTP_PORT);

// Client management
TCPClient tcpClients[MAX_TCP_CLIENTS];
WiFiStatus currentWiFiStatus = WIFI_DISCONNECTED;
String deviceIP = "";
String deviceSSID = "";

void initWiFi() {
    Serial.println("Initializing WiFi...");
    
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
    } else {
        Serial.println("SPIFFS initialized successfully");
    }
    
    // Initialize client array
    for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
        tcpClients[i].connected = false;
        tcpClients[i].lastActivity = 0;
    }
    
    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    
    // Configure WiFiManager
    wifiManager.setConfigPortalTimeout(300); // 5 minutes timeout
    wifiManager.setConnectTimeout(30);       // 30 seconds connect timeout
    wifiManager.setSaveConfigCallback([]() {
        Serial.println("WiFi configuration saved");
    });
    
    // Set custom AP name and password
    wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 4, 1), 
                                   IPAddress(192, 168, 4, 1), 
                                   IPAddress(255, 255, 255, 0));
    
    currentWiFiStatus = WIFI_CONNECTING;
    
    // Try to connect with saved credentials, or start config portal
    if (wifiManager.autoConnect(WIFI_MANAGER_AP_NAME, WIFI_MANAGER_AP_PASS)) {
        Serial.println("WiFi connected successfully");
        deviceIP = WiFi.localIP().toString();
        deviceSSID = WiFi.SSID();
        currentWiFiStatus = WIFI_CONNECTED;
        
        Serial.print("IP address: ");
        Serial.println(deviceIP);
        Serial.print("SSID: ");
        Serial.println(deviceSSID);
        
        startTCPServers();
        setupWebServer();
    } else {
        Serial.println("Failed to connect and hit timeout");
        currentWiFiStatus = WIFI_ERROR;
    }
}

void handleWiFi() {
    // Check WiFi connection status
    if (WiFi.status() != WL_CONNECTED && currentWiFiStatus == WIFI_CONNECTED) {
        Serial.println("WiFi connection lost");
        currentWiFiStatus = WIFI_DISCONNECTED;
        deviceIP = "";
        return;
    }
    
    if (WiFi.status() == WL_CONNECTED && currentWiFiStatus != WIFI_CONNECTED) {
        Serial.println("WiFi reconnected");
        deviceIP = WiFi.localIP().toString();
        deviceSSID = WiFi.SSID();
        currentWiFiStatus = WIFI_CONNECTED;
        startTCPServers();
    }
    
    // Handle TCP clients and web server only if connected
    if (currentWiFiStatus == WIFI_CONNECTED) {
        handleTCPClients();
        webServer.handleClient();
    }
}

void startTCPServers() {
    Serial.println("Starting TCP servers...");
    
    scpiServer.begin();
    telnetServer.begin();
    
    Serial.print("SCPI server started on port ");
    Serial.println(SCPI_TCP_PORT);
    Serial.print("Telnet server started on port ");
    Serial.println(TELNET_PORT);
}

void setupWebServer() {
    webServer.on("/", handleRoot);
    webServer.on("/status", handleStatus);
    webServer.on("/control", HTTP_POST, handleControl);
    webServer.on("/reset", HTTP_POST, handleReset);
    webServer.onNotFound(handleStaticFileOrNotFound);
    
    webServer.begin();
    Serial.print("HTTP server started on port ");
    Serial.println(HTTP_PORT);
}

void handleTCPClients() {
    // Check for new SCPI connections
    WiFiClient newClient = scpiServer.available();
    if (newClient) {
        // Find empty slot for new client
        for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
            if (!tcpClients[i].connected) {
                tcpClients[i].client = newClient;
                tcpClients[i].connected = true;
                tcpClients[i].lastActivity = millis();
                Serial.print("New SCPI client connected: ");
                Serial.println(i);
                newClient.println("# Digital Attenuator SCPI Interface");
                break;
            }
        }
    }
    
    // Check for new Telnet connections
    WiFiClient newTelnetClient = telnetServer.available();
    if (newTelnetClient) {
        // Find empty slot for new client
        for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
            if (!tcpClients[i].connected) {
                tcpClients[i].client = newTelnetClient;
                tcpClients[i].connected = true;
                tcpClients[i].lastActivity = millis();
                Serial.print("New Telnet client connected: ");
                Serial.println(i);
                newTelnetClient.println("Digital Attenuator Telnet Interface");
                newTelnetClient.print("Current attenuation: ");
                newTelnetClient.print(attenuator.getAttenuation());
                newTelnetClient.println(" dB");
                break;
            }
        }
    }
    
    // Handle existing clients
    for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
        if (tcpClients[i].connected) {
            WiFiClient& client = tcpClients[i].client;
            
            // Check if client is still connected
            if (!client.connected()) {
                Serial.print("Client disconnected: ");
                Serial.println(i);
                disconnectClient(i);
                continue;
            }
            
            // Process incoming data
            if (client.available()) {
                tcpClients[i].lastActivity = millis();
                
                // Use SCPI parser for the client
                scpi_attenuator.ProcessInput(client, "\r\n");
            }
            
            // Timeout inactive clients (10 minutes)
            if (millis() - tcpClients[i].lastActivity > 600000) {
                Serial.print("Client timeout: ");
                Serial.println(i);
                client.println("# Connection timeout");
                disconnectClient(i);
            }
        }
    }
}

void disconnectClient(int clientIndex) {
    if (clientIndex >= 0 && clientIndex < MAX_TCP_CLIENTS) {
        tcpClients[clientIndex].client.stop();
        tcpClients[clientIndex].connected = false;
        tcpClients[clientIndex].lastActivity = 0;
    }
}

WiFiStatus getWiFiStatus() {
    return currentWiFiStatus;
}

String getConnectionInfo() {
    String info = "WiFi Status: ";
    switch (currentWiFiStatus) {
        case WIFI_CONNECTED:
            info += "Connected\nSSID: " + deviceSSID + "\nIP: " + deviceIP;
            break;
        case WIFI_CONNECTING:
            info += "Connecting...";
            break;
        case WIFI_DISCONNECTED:
            info += "Disconnected";
            break;
        case WIFI_AP_MODE:
            info += "AP Mode";
            break;
        case WIFI_ERROR:
            info += "Error";
            break;
    }
    return info;
}

void resetWiFiSettings() {
    Serial.println("Resetting WiFi settings...");
    wifiManager.resetSettings();
    ESP.restart();
}

// Helper function to load file from SPIFFS
String loadFile(const String& filename) {
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("Failed to open file: " + filename);
        return "";
    }
    
    String content = file.readString();
    file.close();
    return content;
}

// Helper function to get MIME type based on file extension
String getMimeType(const String& filename) {
    if (filename.endsWith(".html") || filename.endsWith(".htm")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".json")) {
        return "application/json";
    } else if (filename.endsWith(".png")) {
        return "image/png";
    } else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
        return "image/jpeg";
    } else if (filename.endsWith(".gif")) {
        return "image/gif";
    } else if (filename.endsWith(".svg")) {
        return "image/svg+xml";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".pdf")) {
        return "application/pdf";
    } else if (filename.endsWith(".zip")) {
        return "application/zip";
    } else if (filename.endsWith(".xml")) {
        return "text/xml";
    } else if (filename.endsWith(".txt")) {
        return "text/plain";
    } else {
        return "application/octet-stream";  // Default binary type
    }
}

// Web interface handlers
void handleRoot() {
    // Try to serve from SPIFFS first
    String html = loadFile("/index.html");
    
    if (html.length() > 0) {
        webServer.send(200, "text/html", html);
    } else {
        // Fallback to simple HTML if SPIFFS file not found
        String fallback = "<!DOCTYPE html><html><head><title>Digital Attenuator</title></head><body>";
        fallback += "<h1>Digital Attenuator Control</h1>";
        fallback += "<p>Current Attenuation: " + String(attenuator.getAttenuation()) + " dB</p>";
        fallback += "<p>WiFi: " + deviceSSID + " (" + deviceIP + ")</p>";
        fallback += "<form action='/control' method='post'>";
        fallback += "Set Attenuation: <input type='number' name='attenuation' min='0' max='127' value='" + String(attenuator.getAttenuation()) + "'>";
        fallback += "<button type='submit'>Set</button></form>";
        fallback += "<p><a href='/status'>Status JSON</a></p>";
        fallback += "</body></html>";
        webServer.send(200, "text/html", fallback);
    }
}

void handleStatus() {
    String json = "{";
    json += "\"attenuation\":" + String(attenuator.getAttenuation()) + ",";
    json += "\"min_attenuation\":" + String((int)MIN_ATTENUATION) + ",";
    json += "\"max_attenuation\":" + String((int)MAX_ATTENUATION) + ",";
    json += "\"wifi_connected\":" + String(currentWiFiStatus == WIFI_CONNECTED ? "true" : "false") + ",";
    json += "\"wifi_ssid\":\"" + deviceSSID + "\",";
    json += "\"ip_address\":\"" + deviceIP + "\",";
    json += "\"scpi_port\":" + String(SCPI_TCP_PORT) + ",";
    json += "\"telnet_port\":" + String(TELNET_PORT);
    json += "}";
    
    webServer.send(200, "application/json", json);
}

void handleControl() {
    if (webServer.hasArg("attenuation")) {
        int newAttenuation = webServer.arg("attenuation").toInt();
        if (attenuator.setAttenuation(newAttenuation)) {
            webServer.sendHeader("Location", "/");
            webServer.send(302);
            return;
        }
    } else if (webServer.hasArg("action")) {
        String action = webServer.arg("action");
        if (action == "increment") {
            attenuator.increaseAttenuation();
        } else if (action == "decrement") {
            attenuator.decreaseAttenuation();
        }
        webServer.sendHeader("Location", "/");
        webServer.send(302);
        return;
    }
    
    webServer.send(400, "text/plain", "Invalid request");
}

void handleReset() {
    // Try to serve from SPIFFS first
    String html = loadFile("/reset.html");
    
    if (html.length() > 0) {
        webServer.send(200, "text/html", html);
    } else {
        // Fallback HTML if SPIFFS file not found
        webServer.send(200, "text/html", 
            "<!DOCTYPE html><html><head><title>Resetting...</title>"
            "<meta http-equiv='refresh' content='10;url=/'/></head>"
            "<body><h1>Resetting WiFi settings...</h1>"
            "<p>Device will restart. Reconnect to access point \"" WIFI_MANAGER_AP_NAME "\" to reconfigure.</p>"
            "</body></html>");
    }
    
    delay(1000);
    resetWiFiSettings();
}

// Generic static file handler - handles any static file with appropriate MIME type
void handleStaticFileOrNotFound() {
    String requestPath = webServer.uri();
    
    // Skip if it starts with double slash (malformed request)
    if (requestPath.startsWith("//")) {
        webServer.send(400, "text/plain", "Bad Request");
        return;
    }
    
    // Try to serve static file from SPIFFS
    String filename = requestPath;
    
    // Ensure filename starts with /
    if (!filename.startsWith("/")) {
        filename = "/" + filename;
    }
    
    Serial.print("Attempting to serve static file: ");
    Serial.println(filename);
    
    String content = loadFile(filename);
    
    if (content.length() > 0) {
        String mimeType = getMimeType(filename);
        Serial.print("Serving file with MIME type: ");
        Serial.println(mimeType);
        
        webServer.send(200, mimeType, content);
    } else {
        // File not found
        Serial.print("File not found: ");
        Serial.println(filename);
        webServer.send(404, "text/plain", "File Not Found: " + requestPath);
    }
}
