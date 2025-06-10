#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32  
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin assignments for ESP32
const int joystickButton = 25;    // Button input (digital) on GPIO25
const int joystickVRy = 34;       // Vertical axis (analog) on GPIO34

const unsigned long BIOS_TIMEOUT = 4000;

//-----------------------------
// Forward declarations
//-----------------------------
void loadUEFI();
void showBIOS();
void uefiMenu();
void showSysInfo();
void showHardwareSpecs();
void showDiagnostics();
void exitAndSave();

//-----------------------------
// Button debounce function
//-----------------------------
bool buttonPressed() {
  static unsigned long lastDebounce = 0;
  const unsigned long debounceDelay = 300;
  if (digitalRead(joystickButton) == LOW && (millis() - lastDebounce > debounceDelay)) {
    lastDebounce = millis();
    return true;
  }
  return false;
}

//─────────────────────────────
// BIOS Screen (Boot Screen)
//─────────────────────────────
void showBIOS() {
  while (true) {
    display.clearDisplay();
    // Draw a simple logo rectangle in the upper right
    display.fillRect(110, 2, 10, 10, WHITE);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(30, 10);
    display.println("Test UEFI");
    
    display.setCursor(0, 24);
    display.println("Press btn for UEFI");
    display.display();

    unsigned long startTime = millis();
    bool btnDetected = false;
    while (millis() - startTime < BIOS_TIMEOUT) {
      if (digitalRead(joystickButton) == LOW) {
        btnDetected = true;
        break;
      }
      delay(10);
    }
    if (btnDetected) {
      delay(300);
      loadUEFI();
      break;
    }
  }
}

//─────────────────────────────
// UEFI Menu
//─────────────────────────────
enum MenuOption { SYSTEM_INFO, HARDWARE_SPECS, DIAGNOSTICS, EXIT_SAVE };
const int MENU_OPTIONS_COUNT = 4;

void uefiMenu() {
  int currentOption = 0;
  while (true) {
    display.clearDisplay();
    
    // Top bar with version information
    display.fillRect(0, 0, SCREEN_WIDTH, 8, WHITE);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(2, 1);
    display.println("UEFI v0.100.3");

    // Because the screen shows 3 menu lines and we have 4 options:
    int topIndex = (currentOption < MENU_OPTIONS_COUNT - 2) ? 0 : MENU_OPTIONS_COUNT - 3;
    const char* menuItems[MENU_OPTIONS_COUNT] = {
      "1. Sys Info", "2. Hardware", "3. Diagnostics", "4. Exit & Save"
    };

    for (int i = 0; i < 3; i++) {
      int optionIndex = topIndex + i;
      int yPos = 9 + i * 8;
      if (optionIndex == currentOption) {
        // Highlight current option with an inverted rectangle
        display.fillRect(0, yPos, SCREEN_WIDTH, 8, WHITE);
        display.setTextColor(BLACK);
      } else {
        display.setTextColor(WHITE);
      }
      display.setCursor(0, yPos);
      display.println(menuItems[optionIndex]);
    }
    display.display();

    // --- Dead Zone Implementation ---
    // ESP32 ADC is 12-bit (0-4095), so center is approximately 2048.
    int centerVal = 2048;  // Adjust if your hardware deviates.
    int deadZone = 100;    // Values within ±100 of center are ignored.
    int vVal = analogRead(joystickVRy);
    if (vVal > (centerVal + deadZone)) {  
      currentOption = (currentOption + 1) % MENU_OPTIONS_COUNT;
      delay(300);
    } else if (vVal < (centerVal - deadZone)) {  
      currentOption = (currentOption - 1 + MENU_OPTIONS_COUNT) % MENU_OPTIONS_COUNT;
      delay(300);
    }

    if (buttonPressed()) {
      delay(300);
      switch (currentOption) {
        case SYSTEM_INFO:
          showSysInfo();
          break;
        case HARDWARE_SPECS:
          showHardwareSpecs();
          break;
        case DIAGNOSTICS:
          showDiagnostics();
          break;
        case EXIT_SAVE:
          exitAndSave();
          return;  // End UEFI menu after exit & save.
      }
    }
  }
}

//─────────────────────────────
// System Information Screen
//─────────────────────────────
void showSysInfo() {
  while (true) {
    display.clearDisplay();
    // Top bar
    display.fillRect(0, 0, SCREEN_WIDTH, 8, WHITE);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(2, 1);
    display.println("Sys Info");

    // Details
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Board: arduino_board");
    display.setCursor(0, 18);
    display.println("UEFI: v0.100.3");
    display.setCursor(0, 26);
    display.println("CPU: unknown");
    display.display();
    
    if (buttonPressed()) {
      delay(300);
      break;
    }
    delay(10);
  }
}

//─────────────────────────────
// Hardware Specifications Screen
//─────────────────────────────
void showHardwareSpecs() {
  while (true) {
    display.clearDisplay();
    // Top bar
    display.fillRect(0, 0, SCREEN_WIDTH, 8, WHITE);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(2, 1);
    display.println("Hardware");

    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("RAM: unknown");
    display.setCursor(0, 18);
    display.println("Flash: unknown");
    display.setCursor(0, 26);
    display.println("Wi-Fi & BT: unknown");
    display.display();

    if (buttonPressed()) {
      delay(300);
      break;
    }
    delay(10);
  }
}

//─────────────────────────────
// Diagnostics Screen with a Simple Glitch Effect
//─────────────────────────────
void showDiagnostics() {
  while (true) {
    display.clearDisplay();
    // Top bar
    display.fillRect(0, 0, SCREEN_WIDTH, 8, WHITE);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(2, 1);
    display.println("Diagnostics");
    
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Self-test: PASS");
    display.setCursor(0, 18);
    if (random(0, 10) > 7) {
      display.println("Temp: ??C");
    } else {
      display.println("Temp: 28C");
    }
    display.setCursor(0, 26);
    display.println("Volt: unknown");
    display.display();
    
    if (buttonPressed()) {
      delay(300);
      break;
    }
    delay(200);
  }
}

//─────────────────────────────
// Exit & Save (Returns to BIOS Screen)
//─────────────────────────────
void exitAndSave() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Exiting & Saving...");
  display.display();
  delay(1500);
  showBIOS();
}

//─────────────────────────────
// UEFI Entry Point
//─────────────────────────────
void loadUEFI() {
  uefiMenu();
}

void setup() {
  pinMode(joystickButton, INPUT_PULLUP);
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (true);
  }
  display.clearDisplay();
  showBIOS();
}

void loop() {
  // Main functionality is handled within our functions.
}
