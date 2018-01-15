#include <M5Stack.h>
#include <M5LoRa.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

#define LORA_CS_PIN   5
#define LORA_RST_PIN  26
#define LORA_IRQ_PIN  36
#define BAND    433E6 // frequency in Hz (433E6, 866E6, 915E6)
#define myUUID "DECA-FBAD-M5ST-ACK0"
// make your own :-)

uint8_t ix = 0;
char buff[200];
String output = "";
uint8_t receivedCount = 0;
uint8_t brightness = 100;
bool lcdSleep = false;

void setup() {
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_C_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  delay(1000);
  M5.begin();
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_IRQ_PIN); // set CS, reset, IRQ pin
  Serial.println(F("LoRa Receiver"));
  M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Lcd.fillScreen(TFT_WHITE);
  M5.Lcd.drawJpgFile(SD, "/LoRaLogoSmall.jpg", 0, 0);
  M5.Lcd.setFreeFont(FSSB12);
  // Select the font: FreeMono18pt7b – see Free_Fonts.h
  M5.Lcd.drawString(F("Receiver"), 85, 18, GFXFF);
  M5.Lcd.setFreeFont(FSS9); // FreeSans9pt7b
  if (!LoRa.begin(BAND)) {
    Serial.println(F("Starting LoRa failed!"));
    M5.Lcd.drawString(F("Starting LoRa failed!"), 24, 57, GFXFF);
    M5.Lcd.drawString(F("Cannot do anything!"), 24, 82, GFXFF);
    M5.Lcd.drawJpgFile(SD, "/XMark20.jpg", 2, 55);
    while (1);
  }
  Serial.println(F("LoRa init succeeded."));
  M5.Lcd.drawString(F("LoRa init succeeded."), 24, 57, GFXFF);
  M5.Lcd.drawJpgFile(SD, "/Check20.jpg", 2, 55);
  M5.Lcd.drawString(F("Listening to packets..."), 24, 82, GFXFF);
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    // print RSSI of packet
    Serial.print(F("Received packet with RSSI "));
    int rssiLvl = LoRa.packetRssi();
    Serial.println(rssiLvl);
    // read packet
    ix = 0;
    output = "";
    while (LoRa.available()) {
      char c = LoRa.read();
      buff[ix++] = c;
      output += (char)c;
      Serial.write((char)c);
    }
    Serial.write('\n');
    buff[ix++] = '\0';
    uint8_t linePos = 110;
    M5.Lcd.fillRect(0, 100, 320, 137, TFT_WHITE);
    M5.Lcd.setFreeFont(FF1);
    // FreeMono9pt7b
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(buff);
    if (!root.success()) {
      Serial.println("parseObject() failed");
      M5.Lcd.setFreeFont(FSSB9);
      M5.Lcd.drawString(F("* parseObject() failed!"), 5, linePos, GFXFF);
      linePos += 20;
      M5.Lcd.drawString(buff, 5, linePos, GFXFF);
      return;
    }
    const char* from = root["from"];
    const char* msg = root["msg"];
    const char* time = root["time"];
    const char* sendCount = root["sendCount"];
    // Print values.
    Serial.print(F("from: "));
    Serial.println(from);
    Serial.print(F("sendCount: "));
    Serial.println(sendCount);
    Serial.print(F("msg: "));
    Serial.println(msg);

    M5.Lcd.setFreeFont(FSSB9);
    M5.Lcd.drawString(F("* from: "), 5, linePos, GFXFF);
    linePos += 20;
    M5.Lcd.drawString(F("* count: "), 5, linePos, GFXFF);
    linePos += 20;
    M5.Lcd.drawString(F("* msg: "), 5, linePos, GFXFF);
    linePos += 20;
    M5.Lcd.drawString(F("* time: "), 5, linePos, GFXFF);
    linePos += 20;
    M5.Lcd.drawString(F("* RSSI: "), 5, linePos, GFXFF);
    M5.Lcd.setFreeFont(FSS9);
    linePos -= 80;
    M5.Lcd.drawString(from, 100, linePos, GFXFF);
    linePos += 20;
    M5.Lcd.drawString(sendCount, 100, linePos, GFXFF);
    linePos += 20;
    M5.Lcd.drawString(msg, 100, linePos, GFXFF);
    linePos += 20;
    M5.Lcd.drawString(time, 100, linePos, GFXFF);
    linePos += 20;
    M5.Lcd.drawString(String(rssiLvl), 100, linePos, GFXFF);
    String fp;
    if (rssiLvl > -51) fp = "/rssiExcellent40.jpg";
    else if (rssiLvl > -61) fp = "/rssiGood40.jpg";
    else if (rssiLvl > -71) fp = "/rssiFair40.jpg";
    else fp = "/rssiPoor40.jpg";
    M5.Lcd.drawJpgFile(SD, fp.c_str(), 280, 0);
    uint16_t ch0, ch7;
    float vCh0, vCh7;
    ch0 = analogRead(ADC1_CH0);
    ch7 = analogRead(ADC1_CH7);
    Serial.print("ADC1_CH7: ");
    Serial.print(ch7); Serial.write(' ');
    vCh7 = ch7 / 1023.0 * 2.1;
    Serial.println(vCh7);
    Serial.print("ADC1_CH0: ");
    Serial.print(ch0); Serial.write(' ');
    vCh0 = ch0 / 1023.0 * 2.1;
    Serial.println(vCh0);
  }
}
