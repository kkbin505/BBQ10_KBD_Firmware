#include "ConfigManager.h"
#include <BleKeyboard.h> // for keycodes if needed

char ConfigManager::keyboard[CM_COL_COUNT][CM_ROW_COUNT];
char ConfigManager::keyboardSymbol[CM_COL_COUNT][CM_ROW_COUNT];
uint8_t ConfigManager::keyboardLayer3[CM_COL_COUNT][CM_ROW_COUNT];
uint8_t ConfigManager::keyboardSpecial[CM_COL_COUNT][CM_ROW_COUNT];
std::vector<LongPressKey> ConfigManager::longPressKeys;

void ConfigManager::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }
    loadConfig();
}

void ConfigManager::setDefaults() {
    const char defaultKeyboard[CM_COL_COUNT][CM_ROW_COUNT] = {
        {'q', 'w', '\0', 'a', '\0', ' ', '0'}, {'e', 's', 'd', 'p', 'x', 'z', '\0'},
        {'r', 'g', 't', '\0', 'v', 'c', 'f'},  {'u', 'h', 'y', '\0', 'b', 'n', 'j'},
        {'o', 'l', 'i', '\0', '$', 'm', 'k'}
    };
    const char defaultKeyboardSymbol[CM_COL_COUNT][CM_ROW_COUNT] = {
        {'#', '1', '\0', '*', '\0', '\0', '0'},
        {'2', '4', '5', '@', '8', '7', '\0'},
        {'3', '/', '(', '\0', '?', '9', '6'},
        {'_', ':', ')', '\0', '!', ',', ';'},
        {'+', '"', '-', '\0', '\0', '.', '\''}
    };
    const uint8_t defaultKeyboardLayer3[CM_COL_COUNT][CM_ROW_COUNT] = {
        {'\0', KEY_UP_ARROW, '\0', KEY_LEFT_ARROW, '\0', '\0', '\0'},
        {'\0', KEY_DOWN_ARROW, KEY_RIGHT_ARROW, '\0', '\0', '\0', '\0'},
        {'\0', '\0', '\0', '\0', '\0', '\0', '\0'},
        {'\0', '\0', '\0', '\0', '\0', '\0', '\0'},
        {'\0', '\0', '\0', '\0', '\0', '\0', '\0'}
    };
    const uint8_t defaultKeyboardSpecial[CM_COL_COUNT][CM_ROW_COUNT] = {
        {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0}
    };

    memcpy(keyboard, defaultKeyboard, sizeof(keyboard));
    memcpy(keyboardSymbol, defaultKeyboardSymbol, sizeof(keyboardSymbol));
    memcpy(keyboardLayer3, defaultKeyboardLayer3, sizeof(keyboardLayer3));
    memcpy(keyboardSpecial, defaultKeyboardSpecial, sizeof(keyboardSpecial));

    longPressKeys.clear();
    // Default shortcuts
    longPressKeys.push_back({1, 2, 'd', 'f', '5', '6', false, false, 0, 0});
    longPressKeys.push_back({3, 1, 'h', 'j', ':', ';', false, false, 0, 0});
    longPressKeys.push_back({4, 1, 'l', 'k', '"', '\'', false, false, 0, 0});
    longPressKeys.push_back({0, 4, '\0', 0x09, '\0', 0x09, false, false, 0, 0}); // ALT -> Tab
}

void ConfigManager::loadConfig() {
    setDefaults(); // Load defaults first

    File file = LittleFS.open("/config.json", "r");
    if (!file) {
        Serial.println("No config.json found, using defaults");
        saveConfig(); // Create the default config file
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Failed to parse config.json, using defaults");
        file.close();
        return;
    }

    // Load matrices
    JsonArray baseLayer = doc["layers"]["base"];
    for (size_t c = 0; c < CM_COL_COUNT; c++) {
        for (size_t r = 0; r < CM_ROW_COUNT; r++) {
            if (baseLayer[c][r].is<const char*>()) {
                const char* str = baseLayer[c][r].as<const char*>();
                keyboard[c][r] = (str && str[0]) ? str[0] : '\0';
            }
        }
    }

    JsonArray symbolLayer = doc["layers"]["symbol"];
    for (size_t c = 0; c < CM_COL_COUNT; c++) {
        for (size_t r = 0; r < CM_ROW_COUNT; r++) {
            if (symbolLayer[c][r].is<const char*>()) {
                const char* str = symbolLayer[c][r].as<const char*>();
                keyboardSymbol[c][r] = (str && str[0]) ? str[0] : '\0';
            }
        }
    }

    JsonArray layer3 = doc["layers"]["layer3"];
    for (size_t c = 0; c < CM_COL_COUNT; c++) {
        for (size_t r = 0; r < CM_ROW_COUNT; r++) {
            if (layer3[c][r].is<uint8_t>()) {
                keyboardLayer3[c][r] = layer3[c][r].as<uint8_t>();
            }
        }
    }

    // Load long press keys
    JsonArray lpks = doc["shortcuts"];
    if (!lpks.isNull()) {
        longPressKeys.clear();
        for (JsonObject lpk : lpks) {
            LongPressKey key = {0};
            key.col = lpk["col"] | 0;
            key.row = lpk["row"] | 0;
            
            const char* bsc = lpk["baseShort"];
            key.baseShortOutput = (bsc && bsc[0]) ? bsc[0] : (lpk["baseShortInt"] | 0);
            
            const char* blc = lpk["baseLong"];
            key.baseLongOutput = (blc && blc[0]) ? blc[0] : (lpk["baseLongInt"] | 0);
            
            const char* l2sc = lpk["layer2Short"];
            key.layer2ShortOutput = (l2sc && l2sc[0]) ? l2sc[0] : (lpk["layer2ShortInt"] | 0);
            
            const char* l2lc = lpk["layer2Long"];
            key.layer2LongOutput = (l2lc && l2lc[0]) ? l2lc[0] : (lpk["layer2LongInt"] | 0);
            
            longPressKeys.push_back(key);
        }
    }

    file.close();
}

void ConfigManager::saveConfig() {
    JsonDocument doc;

    JsonArray baseLayer = doc["layers"]["base"].to<JsonArray>();
    for (size_t c = 0; c < CM_COL_COUNT; c++) {
        JsonArray row = baseLayer.add<JsonArray>();
        for (size_t r = 0; r < CM_ROW_COUNT; r++) {
            if (keyboard[c][r] == '\0') row.add(nullptr);
            else {
                char str[2] = {keyboard[c][r], '\0'};
                row.add(str);
            }
        }
    }

    JsonArray symbolLayer = doc["layers"]["symbol"].to<JsonArray>();
    for (size_t c = 0; c < CM_COL_COUNT; c++) {
        JsonArray row = symbolLayer.add<JsonArray>();
        for (size_t r = 0; r < CM_ROW_COUNT; r++) {
            if (keyboardSymbol[c][r] == '\0') row.add(nullptr);
            else {
                char str[2] = {keyboardSymbol[c][r], '\0'};
                row.add(str);
            }
        }
    }

    JsonArray layer3 = doc["layers"]["layer3"].to<JsonArray>();
    for (size_t c = 0; c < CM_COL_COUNT; c++) {
        JsonArray row = layer3.add<JsonArray>();
        for (size_t r = 0; r < CM_ROW_COUNT; r++) {
            row.add(keyboardLayer3[c][r]);
        }
    }

    JsonArray lpks = doc["shortcuts"].to<JsonArray>();
    for (const auto& key : longPressKeys) {
        JsonObject obj = lpks.add<JsonObject>();
        obj["col"] = key.col;
        obj["row"] = key.row;
        
        if (key.baseShortOutput >= 32 && key.baseShortOutput <= 126) {
            char str[2] = {key.baseShortOutput, '\0'};
            obj["baseShort"] = str;
        } else obj["baseShortInt"] = (uint8_t)key.baseShortOutput;

        if (key.baseLongOutput >= 32 && key.baseLongOutput <= 126) {
            char str[2] = {key.baseLongOutput, '\0'};
            obj["baseLong"] = str;
        } else obj["baseLongInt"] = (uint8_t)key.baseLongOutput;

        if (key.layer2ShortOutput >= 32 && key.layer2ShortOutput <= 126) {
            char str[2] = {key.layer2ShortOutput, '\0'};
            obj["layer2Short"] = str;
        } else obj["layer2ShortInt"] = (uint8_t)key.layer2ShortOutput;

        if (key.layer2LongOutput >= 32 && key.layer2LongOutput <= 126) {
            char str[2] = {key.layer2LongOutput, '\0'};
            obj["layer2Long"] = str;
        } else obj["layer2LongInt"] = (uint8_t)key.layer2LongOutput;
    }

    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    serializeJson(doc, file);
    file.close();
}
