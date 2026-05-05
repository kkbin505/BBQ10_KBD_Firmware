import re

with open('src/main.cpp', 'r', encoding='utf-8') as f:
    code = f.read()

# 1. Add include
if '#include "ConfigManager.h"' not in code:
    code = code.replace('#include <USBHIDKeyboard.h>\n#undef KeyReport\n', '#include <USBHIDKeyboard.h>\n#undef KeyReport\n\n#include "ConfigManager.h"\n')

# 2. Remove const arrays
code = re.sub(r'// \[col\]\[row\].*?};\n\n?', '', code, flags=re.DOTALL)

# 3. Remove LongPressKey struct and array
code = re.sub(r'struct LongPressKey \{.*?LongPressKey longPressKeys\[\] = \{.*?};\n\n?', '', code, flags=re.DOTALL)

# 4. Replace variable usages
code = code.replace('keyboard[colIndex][rowIndex]', 'ConfigManager::keyboard[colIndex][rowIndex]')
code = code.replace('keyboardSymbol[colIndex][rowIndex]', 'ConfigManager::keyboardSymbol[colIndex][rowIndex]')
code = code.replace('keyboardLayer3[colIndex][rowIndex]', 'ConfigManager::keyboardLayer3[colIndex][rowIndex]')
code = code.replace('keyboardSpecial[colIndex][rowIndex]', 'ConfigManager::keyboardSpecial[colIndex][rowIndex]')

code = code.replace('(sizeof(longPressKeys) / sizeof(longPressKeys[0]))', 'ConfigManager::longPressKeys.size()')
code = code.replace('longPressKeys[i]', 'ConfigManager::longPressKeys[i]')

# 5. Add ConfigManager::begin() to setup
if 'ConfigManager::begin();' not in code:
    code = code.replace('void setup() {\n', 'void setup() {\n  Serial.begin(115200);\n  ConfigManager::begin();\n')

with open('src/main.cpp', 'w', encoding='utf-8') as f:
    f.write(code)

print("Done refactoring main.cpp.")
