#include <Wire.h>
#include <Adafruit_PN532.h>

#define SDA_PIN 20 
#define SCL_PIN 21

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

// Standard default key for Mifare Classic 1K
uint8_t keyA[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

void setup(void) {
  Serial.begin(115200);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("PN532 board not found!");
    while (1);
  }
  nfc.SAMConfig();
  Serial.println("System Ready. Tap card to RESET balance to 0...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  // Detect NFC Card
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("Card detected... attempting to clear balance.");
    
    // 1. Authenticate to Sector 1 (Block 4)
    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keyA);
    
    if (success) {
      // 2. Prepare "Zero" Data (16 bytes of zeros)
      uint8_t zeroData[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      
      // 3. Write zeros to Block 4
      success = nfc.mifareclassic_WriteDataBlock(4, zeroData);
      
      if (success) {
        Serial.println("--------------------------------");
        Serial.println("SUCCESS: Balance is now 0 PHP!");
        Serial.println("--------------------------------");
      } else {
        Serial.println("ERROR: Write failed. Card might be locked.");
      }
    } else {
      Serial.println("AUTH ERROR: Access denied. Check your Key A.");
    }
    
    // Prevent rapid re-reading
    delay(3000);
    Serial.println("\nReady for next card...");
  }
}