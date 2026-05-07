#include "WebManager.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "ConfigManager.h"
#include <BleKeyboard.h>

extern BleKeyboard Keyboard;

bool WebManager::_configMode = false;
DNSServer WebManager::_dnsServer;
static AsyncWebServer server(80);

void WebManager::begin() {
    // Initial setup if needed
}

bool WebManager::isConfigMode() {
    return _configMode;
}

void WebManager::startConfigMode() {
    if (_configMode) return;
    _configMode = true;

    Serial.println("Entering Config Mode...");
    
    // Stop BLE
    if (Keyboard.isConnected()) {
        Keyboard.end();
    }
    
    // Start WiFi AP
    WiFi.mode(WIFI_AP);
    // Setting IP to 10.0.0.1 as requested
    WiFi.softAPConfig(IPAddress(10, 0, 0, 1), IPAddress(10, 0, 0, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP("BBQ10_Config", "12345678");

    // Start DNS Server (redirect all domains to AP IP)
    // This creates the "Captive Portal" effect
    _dnsServer.start(53, "*", IPAddress(10, 0, 0, 1));

    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // Setup routes
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // Common Captive Portal endpoints to force redirection
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){ request->redirect("/"); });
    server.on("/connectivity-check", HTTP_GET, [](AsyncWebServerRequest *request){ request->redirect("/"); });
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request){ request->redirect("/"); });
    server.on("/canonical.html", HTTP_GET, [](AsyncWebServerRequest *request){ request->redirect("/"); });
    server.on("/success.txt", HTTP_GET, [](AsyncWebServerRequest *request){ request->send(200, "text/plain", "success"); });
    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request){ request->send(200, "text/plain", "Microsoft NCSI"); });
    server.on("/redirect", HTTP_GET, [](AsyncWebServerRequest *request){ request->redirect("/"); });

    server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request){
        if (!LittleFS.exists("/config.json")) {
            request->send(404, "application/json", "{\"error\":\"Config not found\"}");
            return;
        }
        request->send(LittleFS, "/config.json", "application/json");
    });

    server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request){
        // Handled by body handler below
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        static String body = "";
        if (index == 0) body = "";
        for (size_t i = 0; i < len; i++) body += (char)data[i];
        
        if (index + len == total) {
            File file = LittleFS.open("/config.json", "w");
            if (file) {
                file.print(body);
                file.close();
                ConfigManager::loadConfig();
                request->send(200, "application/json", "{\"status\":\"success\"}");
            } else {
                request->send(500, "application/json", "{\"error\":\"Failed to write\"}");
            }
        }
    });

    server.on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "application/json", "{\"status\":\"rebooting\"}");
        delay(1000);
        ESP.restart();
    });

    // Catch-all: Redirect any unknown path to root (Captive Portal requirement)
    server.onNotFound([](AsyncWebServerRequest *request){
        request->redirect("/");
    });

    server.begin();
    Serial.println("Web server and Captive Portal DNS started.");
}

void WebManager::handle() {
    if (_configMode) {
        _dnsServer.processNextRequest();
    }
}
