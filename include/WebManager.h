#pragma once

#include <Arduino.h>
#include <DNSServer.h>

class WebManager {
public:
    static void begin();
    static void startConfigMode();
    static bool isConfigMode();
    static void handle();

private:
    static bool _configMode;
    static DNSServer _dnsServer;
};
