/*
 * ESP8266 Receiver for BT2UART Bridge
 *
 * Receives HID report frames from Pico W via UART and displays keystrokes.
 *
 * Hardware Connections:
 * - Pico W GP4 (TX) -> ESP8266 RX (4th pin from USB: 3v3, GND, TX, RX)
 * - Pico W GND -> ESP8266 GND (2nd pin from USB)
 *
 * UART Settings: 115200 baud, 8N1
 *
 * Protocol: [0xFE] [8-byte HID report]
 * - Byte 0: 0xFE sync marker
 * - Byte 1: Modifier keys (bitmask)
 * - Byte 2: Reserved (always 0)
 * - Bytes 3-8: Key codes (up to 6 simultaneous keys)
 */

#include <Arduino.h>

// UART configuration - Use Serial1 for hardware UART on ESP8266
#define BAUD_RATE 115200
#define HID_REPORT_SIZE 8
#define SYNC_BYTE 0xFE

// ESP8266 GPIO pins (use dedicated RX pin)
#define UART_RX_PIN 3   // GPIO3 = RX pin (4th from USB: 3v3, GND, TX, RX)
#define UART_TX_PIN 15  // GPIO15 = TX pin (not used, but required for Serial1)

// Modifier key bitmasks
#define MOD_LCTRL   0x01
#define MOD_LSHIFT  0x02
#define MOD_LALT    0x04
#define MOD_LGUI    0x08
#define MOD_RCTRL   0x10
#define MOD_RSHIFT  0x20
#define MOD_RALT    0x40
#define MOD_RGUI    0x80

// HID key codes to ASCII mapping (simplified - common keys only)
const char* get_key_name(uint8_t key_code) {
  static const char* key_names[] = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
    "Enter", "Esc", "Backspace", "Tab", "Space",
    "-", "=", "[", "]", "Pipe", ";", "'", "`",  // Pipe instead of backslash to avoid escaping issues
    ",", ".", "/", "CapsLock",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
    "PrintScreen", "ScrollLock", "Pause", "Insert", "Home", "PageUp", "Delete", "End", "PageDown",
    "Right", "Left", "Down", "Up"
  };

  if (key_code == 0) return "None";
  if (key_code >= 0x04 && key_code <= 0x65) {
    return key_names[key_code - 0x04];
  }
  return "Unknown";
}

// Convert HID key code to printable character (basic implementation)
char hid_to_char(uint8_t key_code, uint8_t modifiers) {
  bool shift = (modifiers & (MOD_LSHIFT | MOD_RSHIFT));

  // Letters A-Z
  if (key_code >= 0x04 && key_code <= 0x1D) {
    return shift ? ('A' + key_code - 0x04) : ('a' + key_code - 0x04);
  }

  // Numbers 1-9, 0
  if (key_code >= 0x1E && key_code <= 0x27) {
    static const char shifted[] = "!@#$%^&*()";
    static const char normal[] = "1234567890";
    return shift ? shifted[key_code - 0x1E] : normal[key_code - 0x1E];
  }

  // Common symbols
  switch (key_code) {
    case 0x2C: return ' ';  // Space
    case 0x2D: return shift ? '_' : '-';  // -/_
    case 0x2E: return shift ? '+' : '=';  //= /+
    case 0x2F: return shift ? '{' : '[';  // [ /{
    case 0x30: return shift ? '}' : ']';  // ] /}
    case 0x31: return shift ? '|' : '\\'; // \ /|
    case 0x33: return shift ? ':' : ';';  // ; /:
    case 0x34: return shift ? '"' : '\''; // '/"
    case 0x35: return shift ? '~' : '`';  // `/~
    case 0x36: return shift ? '<' : ',';  // ,/<
    case 0x37: return shift ? '>' : '.';  // ./>
    case 0x38: return shift ? '?' : '/';  // //?
    case 0x28: return '\r'; // Enter
    case 0x2A: return '\b'; // Backspace
    case 0x2B: return '\t'; // Tab
    case 0x29: return 0x1B; // Escape
  }

  return 0; // No printable character
}

void print_modifiers(uint8_t modifiers) {
  if (modifiers == 0) return;

  Serial.print(" [");
  if (modifiers & MOD_LCTRL)  Serial.print("LCtrl+");
  if (modifiers & MOD_RCTRL)  Serial.print("RCtrl+");
  if (modifiers & MOD_LSHIFT) Serial.print("LShift+");
  if (modifiers & MOD_RSHIFT) Serial.print("RShift+");
  if (modifiers & MOD_LALT)   Serial.print("LAlt+");
  if (modifiers & MOD_RALT)   Serial.print("RAlt+");
  if (modifiers & MOD_LGUI)   Serial.print("LGui+");
  if (modifiers & MOD_RGUI)   Serial.print("RGui+");

  // Remove trailing +
  Serial.print("\b] ");
}

uint8_t frame_buffer[9];  // [SYNC + 8-byte report]
uint8_t buffer_pos = 0;
uint8_t prev_report[8] = {0};

void process_hid_report(const uint8_t *report) {
  // Check if this is a new keypress (compare with previous)
  bool is_new = false;
  for (int i = 0; i < 8; i++) {
    if (report[i] != prev_report[i]) {
      is_new = true;
      break;
    }
  }

  if (!is_new) return; // Skip duplicate reports

  // Print raw HID report
  Serial.print("RAW: ");
  for (int i = 0; i < 8; i++) {
    Serial.printf("%02X ", report[i]);
  }
  Serial.println();

  // Print modifier keys
  uint8_t modifiers = report[0];
  if (modifiers != 0) {
    Serial.print("Modifiers: ");
    print_modifiers(modifiers);
    Serial.println();
  }

  // Process key codes (bytes 2-7)
  for (int i = 2; i < 8; i++) {
    uint8_t key_code = report[i];
    if (key_code != 0 && key_code != prev_report[i]) {
      const char* key_name = get_key_name(key_code);
      char printable = hid_to_char(key_code, modifiers);

      Serial.printf("KEY %s: %s (0x%02X)",
                   (report[i] != 0) ? "press" : "release",
                   key_name, key_code);

      if (printable != 0) {
        Serial.printf(" -> '%c'", printable);
      }
      Serial.println();
    }
  }

  // Save current report for next comparison
  memcpy(prev_report, report, 8);
}

void setup() {
  // Initialize debug serial (USB)
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait up to 3 seconds for serial

  Serial.println("\n\n=== ESP8266 BT2UART Receiver ===");
  Serial.println("Waiting for HID frames from Pico W...");
  Serial.println("Press keys on your BLE keyboard!");
  Serial.println();

  // Initialize hardware UART1 for receiving from Pico W
  // ESP8266 Serial1: RX=GPIO3 (RX pin), TX=GPIO15 (TX pin, not used)
  Serial1.begin(BAUD_RATE);
  Serial1.swap();  // Use alternate pins (RX=GPIO3, TX=GPIO15)

  Serial.println("UART1 initialized on GPIO13 (RX)");
  Serial.println("Ready!");
  Serial.println();
}

void loop() {
  // Read available bytes from UART
  while (Serial1.available() > 0) {
    uint8_t byte = Serial1.read();

    // Look for sync byte
    if (buffer_pos == 0 && byte != SYNC_BYTE) {
      continue; // Keep looking for sync byte
    }

    // Add byte to buffer
    frame_buffer[buffer_pos++] = byte;

    // Check if we have a complete frame
    if (buffer_pos >= 9) {
      // Process the HID report (skip sync byte)
      process_hid_report(frame_buffer + 1);
      buffer_pos = 0; // Reset for next frame
    }
  }

  // Reset buffer if we haven't received a complete frame in 100ms
  // This helps recover from sync errors
  static unsigned long last_byte_time = 0;
  if (buffer_pos > 0 && millis() - last_byte_time > 100) {
    buffer_pos = 0;
  }
  if (Serial1.available() > 0) {
    last_byte_time = millis();
  }
}