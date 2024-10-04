#include "libRfid.h"

MFRC522 mfrc522(RFID_CS_PIN);

/*
RfidManager::RfidManager(uint8_t ssPin, uint8_t rstPin) 
    : ssPin(ssPin), rstPin(rstPin), rfidReader(ssPin, rstPin) {
}

void RfidManager::init() {
    SPI.begin();  // Initialize SPI bus
    rfidReader.PCD_Init();  // Initialize RFID reader
}

bool RfidManager::isCardPresent() {
    return rfidReader.PICC_IsNewCardPresent();
}

bool RfidManager::readCardUid(char* uidBuffer) {
    if (!rfidReader.PICC_IsNewCardPresent() || !rfidReader.PICC_ReadCardSerial()) {
        return false;
    }

    // Convert UID to a readable format (e.g., hex)
    for (byte i = 0; i < rfidReader.uid.size; i++) {
        sprintf(uidBuffer + (i * 2), "%02X", rfidReader.uid.uidByte[i]);
    }
    uidBuffer[rfidReader.uid.size * 2] = '\0'; // Null-terminate the string
    return true;
}

bool RfidManager::compareUid(const char* scannedUid, const char* storedUid) {
    return strcmp(scannedUid, storedUid) == 0;
}

bool RfidManager::invalidateCard(const char* uid) {
    // Logic to invalidate the card and trigger an alarm
    // (This can be expanded based on your alarm system)
    Serial.println("Invalid card detected! Triggering alarm...");
    return true;
}
*/
