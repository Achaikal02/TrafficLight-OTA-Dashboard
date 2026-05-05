#define FIRMWARE_VERSION "1.0"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "kura kura";
const char* password = "ikal1234";

String base_url = "https://ottawa-refresh-accompanied-citizenship.trycloudflare.com";

#define TL1_RED     25
#define TL1_YELLOW  26
#define TL1_GREEN   27
#define TL2_RED     33
#define TL2_YELLOW  32
#define TL2_GREEN   13
#define TL3_RED     23
#define TL3_YELLOW  22
#define TL3_GREEN   12
#define TL4_RED     18
#define TL4_YELLOW  19
#define TL4_GREEN   21

// ================= CEK VERSI =================
bool isNewVersionAvailable() {
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(10000);

  HTTPClient http;
  http.begin(client, base_url + "/version");
  http.addHeader("User-Agent", "ESP32-OTA");
  http.setTimeout(10000);

  int code = http.GET();
  if (code != 200) {
    Serial.printf("Versi cek gagal: %d\n", code);
    http.end();
    return false;
  }

  String serverVersion = http.getString();
  serverVersion.trim();
  http.end();

  Serial.printf("Server: %s | Local: %s\n", serverVersion.c_str(), FIRMWARE_VERSION);

  return serverVersion != String(FIRMWARE_VERSION);
}

// ================= OTA =================
void updateFirmware() {
  if (WiFi.status() != WL_CONNECTED) return;
  Serial.println("Checking OTA...");

  if (!isNewVersionAvailable()) {
    Serial.println("Firmware sudah terbaru, skip.");
    return;
  }

  Serial.println("Update tersedia! Downloading...");

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15000);

  HTTPClient http;
  http.begin(client, base_url + "/firmware.bin");
  http.addHeader("User-Agent", "ESP32-OTA");
  http.setTimeout(15000);

  int httpCode = http.GET();
  Serial.printf("HTTP Code: %d\n", httpCode);

  if (httpCode != 200) {
    Serial.printf("❌ HTTP gagal: %d\n", httpCode);
    http.end();
    return;
  }

  int contentLength = http.getSize();
  Serial.printf("Size: %d bytes\n", contentLength);

  if (contentLength <= 0) {
    Serial.println("❌ Content-Length tidak valid");
    http.end();
    return;
  }

  if (!Update.begin(contentLength)) {
    Serial.println("❌ Tidak cukup ruang");
    http.end();
    return;
  }

  WiFiClient* stream = http.getStreamPtr();
  uint8_t buf[128];
  int totalWritten = 0;
  int timeout = 0;

  while (http.connected() && totalWritten < contentLength) {
    size_t available = stream->available();
    if (available) {
      int bytesRead = stream->readBytes(buf, min((size_t)128, available));
      Update.write(buf, bytesRead);
      totalWritten += bytesRead;
      timeout = 0;
    } else {
      delay(1);
      timeout++;
      if (timeout > 5000) {
        Serial.println("❌ Stream timeout");
        break;
      }
    }
  }

  Serial.printf("Total written: %d\n", totalWritten);

  if (Update.end() && Update.isFinished()) {
    Serial.println("✅ Update sukses! Restarting...");
    delay(1000);
    ESP.restart();
  } else {
    Serial.printf("❌ Update error: %d\n", Update.getError());
  }

  http.end();
}

// ================= SETUP =================
void setup() {
  Serial.begin(9600);

  pinMode(TL1_RED, OUTPUT); pinMode(TL1_YELLOW, OUTPUT); pinMode(TL1_GREEN, OUTPUT);
  pinMode(TL2_RED, OUTPUT); pinMode(TL2_YELLOW, OUTPUT); pinMode(TL2_GREEN, OUTPUT);
  pinMode(TL3_RED, OUTPUT); pinMode(TL3_YELLOW, OUTPUT); pinMode(TL3_GREEN, OUTPUT);
  pinMode(TL4_RED, OUTPUT); pinMode(TL4_YELLOW, OUTPUT); pinMode(TL4_GREEN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(1000);

  WiFi.begin(ssid, password);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  updateFirmware();
}

// ================= HELPER =================
void allRed() {
  digitalWrite(TL1_RED, HIGH); digitalWrite(TL1_YELLOW, LOW); digitalWrite(TL1_GREEN, LOW);
  digitalWrite(TL2_RED, HIGH); digitalWrite(TL2_YELLOW, LOW); digitalWrite(TL2_GREEN, LOW);
  digitalWrite(TL3_RED, HIGH); digitalWrite(TL3_YELLOW, LOW); digitalWrite(TL3_GREEN, LOW);
  digitalWrite(TL4_RED, HIGH); digitalWrite(TL4_YELLOW, LOW); digitalWrite(TL4_GREEN, LOW);
}

unsigned long lastOTA = 0;
const unsigned long OTA_INTERVAL = 30000;

// ================= LOOP =================
void loop() {
  if (millis() - lastOTA >= OTA_INTERVAL) {
    lastOTA = millis();
    updateFirmware();
  }

  // FASE 1: TL1 & TL3 HIJAU
  allRed();
  digitalWrite(TL1_RED, LOW); digitalWrite(TL1_GREEN, HIGH);
  digitalWrite(TL3_RED, LOW); digitalWrite(TL3_GREEN, HIGH);
  delay(5000);

  digitalWrite(TL1_GREEN, LOW); digitalWrite(TL1_YELLOW, HIGH);
  digitalWrite(TL3_GREEN, LOW); digitalWrite(TL3_YELLOW, HIGH);
  delay(2000);

  // FASE 2: TL2 & TL4 HIJAU
  allRed();
  digitalWrite(TL2_RED, LOW); digitalWrite(TL2_GREEN, HIGH);
  digitalWrite(TL4_RED, LOW); digitalWrite(TL4_GREEN, HIGH);
  delay(5000);

  digitalWrite(TL2_GREEN, LOW); digitalWrite(TL2_YELLOW, HIGH);
  digitalWrite(TL4_GREEN, LOW); digitalWrite(TL4_YELLOW, HIGH);
  delay(2000);
}