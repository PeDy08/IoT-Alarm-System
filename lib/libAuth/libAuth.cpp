#include "libAuth.h"

MFRC522 mfrc522(RFID_CS_PIN, RFID_RST_PIN);

String hashPassword(String &inputPassword) {
    esplogI(" (libHash): Hashing password...\n");
    inputPassword.trim();
    uint8_t hash[32]; // SHA-256 produces a 32-byte hash
    mbedtls_sha256_context ctx;
    
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0); // 0 for SHA-256
    mbedtls_sha256_update(&ctx, (const unsigned char *)inputPassword.c_str(), inputPassword.length());
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);

    // Convert hash to a hexadecimal string
    String hashedPassword;
    for (int i = 0; i < 32; i++) {
        hashedPassword += String(hash[i], HEX);
    }
    return hashedPassword;
}

bool checkPassword(String &inputPassword) {
    esplogI(" (libHash): Checking password...\n");
    inputPassword.trim();
    if (!SD.exists(LOCK_FILE)) {
        esplogW(" (libHash): Password file doesn't exist!\n");
        return false;
    }

    File passwordFile = SD.open(LOCK_FILE, "r");
    if (!passwordFile) {
        esplogE(" (libJson): Failed to open password file: '%s'! Unexpected error!\n", LOCK_FILE);
        return false;
    }

    String actualPassword;
    if (inputPassword.endsWith("#")) {
        actualPassword = inputPassword.substring(0, inputPassword.length() - 1);
    } else {
        actualPassword = inputPassword;
    }
    int storedPasswordLength = passwordFile.readStringUntil('\n').toInt();

    if (actualPassword.length() != storedPasswordLength) {
        esplogI(" (libHash): Incorrect password! Password length mismatch!\n");
        passwordFile.close();
        return false;
    }

    String storedPassword = passwordFile.readStringUntil('\n');
    String storedHashedPassword = passwordFile.readStringUntil('\n');
    String hashedPassword = hashPassword(actualPassword);

    storedPassword.trim();
    storedHashedPassword.trim();
    hashedPassword.trim();
    
    passwordFile.close();
    
    if (hashedPassword == storedHashedPassword) {
        esplogI(" (libHash): Password is correct.\n");
        return true;
    } else {
        esplogI(" (libHash): Incorrect password!\n");
        return false;
    }
}

bool savePassword(String &inputPassword) {
    esplogI(" (libHash): Saving password to file...\n");
    inputPassword.trim();
    if (SD.exists(LOCK_FILE)) {
        esplogW(" (libHash): Password file found, rewriting!\n");
        if (!SD.remove(LOCK_FILE)) {
            esplogE(" (libHash): Failed to rewrite existing file: %s!\n", LOCK_FILE);
            return false;
        }
    }

    File passwordFile = SD.open(LOCK_FILE, "w");
    if (!passwordFile) {
        esplogE(" (libHash): Failed to open password file: %s when writing! Unexpected error!\n", LOCK_FILE);
        return false;
    }

    String actualPassword;
    if (inputPassword.endsWith("#")) {
        actualPassword = inputPassword.substring(0, inputPassword.length() - 1);
    } else {
        actualPassword = inputPassword;
    }

    if (actualPassword.length() < MIN_PASSWORD_LENGTH || actualPassword.length() > MAX_PASSWORD_LENGTH) {
        esplogW(" (libHash): Failed to save new password! Password was too short or too long!\n");
        passwordFile.close();
        return false;
    }

    String hashedPassword = hashPassword(actualPassword);
    passwordFile.println(actualPassword.length());
    passwordFile.println(actualPassword);
    passwordFile.println(hashedPassword);
    passwordFile.close();
    return true;
}

bool saveNewPassword(String &inputDoublePassword) {
    esplogI(" (libHash): Saving new password to file...\n");
    inputDoublePassword.trim();
    if (SD.exists(LOCK_FILE)) {
        esplogW(" (libHash): Password file found, rewriting!\n");
        if (!SD.remove(LOCK_FILE)) {
            esplogE(" (libHash): Failed to rewrite existing file: %s!\n", LOCK_FILE);
            return false;
        }
    }

    int firstDelimiterPos = inputDoublePassword.indexOf('#');
    int secondDelimiterPos = inputDoublePassword.lastIndexOf('#');

    if (firstDelimiterPos == -1 || secondDelimiterPos == -1 || firstDelimiterPos == secondDelimiterPos) {
        esplogW(" (libHash): Failed to save new password! Password string parsing error! Unexpected string format: %s", inputDoublePassword);
        return false;
    }

    String firstPassword = inputDoublePassword.substring(0, firstDelimiterPos);
    String secondPassword = inputDoublePassword.substring(firstDelimiterPos+1, secondDelimiterPos);
    
    if (firstPassword != secondPassword) {
        esplogW(" (libHash): Failed to save new password! Different passwords were written on setup!\n - first:  %s\n - second: %s\n", firstPassword, secondPassword);
        return false;
    }

    if (firstPassword.length() < MIN_PASSWORD_LENGTH || firstPassword.length() > MAX_PASSWORD_LENGTH) {
        esplogW(" (libHash): Failed to save new password! Password was too short or too long!\n");
        return false;
    }

    File passwordFile = SD.open(LOCK_FILE, "w");
    if (!passwordFile) {
        esplogE(" (libHash): Failed to open password file: %s when writing! Unexpected error!\n", LOCK_FILE);
        return false;
    }

    String hashedPassword = hashPassword(firstPassword);
    passwordFile.println(firstPassword.length());
    passwordFile.println(firstPassword);
    passwordFile.println(hashedPassword);
    passwordFile.close();
    return true;
}

bool existsPassword() {
    esplogI(" (libHash): Checking if password exists... %d\n", SD.exists(LOCK_FILE));
    return SD.exists(LOCK_FILE);
}

int lengthPassword() {
    esplogI(" (libHash): Computing password length...\n");
    if (!SD.exists(LOCK_FILE)) {
        esplogW(" (libHash): Password file doesn't exist!\n");
        return 0;
    }

    File passwordFile = SD.open(LOCK_FILE, "r");
    if (!passwordFile) {
        esplogE(" (libJson): Failed to open password file: '%s'! Unexpected error!\n", LOCK_FILE);
        return -1;
    }

    int storedPasswordLength = passwordFile.readStringUntil('\n').toInt();
    return storedPasswordLength;
}


// ******************************** RFID ********************************
bool saveRfid(String &inputRfid) {
    esplogI(" (libHash): Saving rfid to file...\n");
    inputRfid.trim();
    if (SD.exists(RFID_FILE)) {
        esplogW(" (libHash): RFID file found, rewriting!\n");
        if (!SD.remove(RFID_FILE)) {
            esplogE(" (libHash): Failed to rewrite existing file: %s!\n", RFID_FILE);
            return false;
        }
    }

    File rfidFile = SD.open(RFID_FILE, "w");
    if (!rfidFile) {
        esplogE(" (libHash): Failed to open RFID file: %s! Unexpected error!\n", RFID_FILE);
        return false;
    }

    // if is needed any check for input RFID UID here needs to be implemented

    rfidFile.println(inputRfid);
    rfidFile.close();
    return true;
}

bool addRfid(String &inputRfid) {
    esplogI(" (libHash): Adding new rfid record to RFID file...\n");
    inputRfid.trim();

    File rfidFile = SD.open(RFID_FILE, "r");
    bool recordFound = false;
    while (rfidFile.available()) {
        String line = rfidFile.readStringUntil('\n');
        line.trim();
        
        if (line == inputRfid) {
            esplogI(" (libHash): Found matching RFID record: %s\n", inputRfid.c_str());
            recordFound = true;
            break;
        }
    }

    rfidFile.close();
    if (recordFound) {
        esplogW(" (libHash): RFID UID already added: %s! Ignoring...\n", inputRfid);
        return true;
    }

    if (!SD.exists(RFID_FILE)) {
        esplogW(" (libHash): RFID file not found! Creating new file!\n");
        rfidFile = SD.open(RFID_FILE, "w");
    } else {
        esplogW(" (libHash): RFID file found! Appending record to existing file!\n");
        rfidFile = SD.open(RFID_FILE, "a");
    }

    if (!rfidFile) {
        esplogE(" (libHash): Failed to open RFID file: %s! Unexpected error!\n", RFID_FILE);
        return false;
    }

    // if is needed any check for input RFID UID here needs to be implemented

    rfidFile.println(inputRfid);
    rfidFile.close();
    return true;
}

bool delRfid(String &inputRfid) {
    esplogI(" (libHash): Deleting rfid record from file...\n");
    inputRfid.trim();
    if (!SD.exists(RFID_FILE)) {
        esplogW(" (libHash): RFID file not found!\n");
        return false;
    }

    File rfidFile = SD.open(RFID_FILE, "r");
    if (!rfidFile) {
        esplogE(" (libHash): Failed to open RFID file: %s! Unexpected error!\n", RFID_FILE);
        return false;
    }

    File tempFile = SD.open(RFID_TMP_FILE, "w");
    if (!tempFile) {
        esplogE(" (libHash): Failed to create temporary file: %s! Unexpected error!\n", RFID_TMP_FILE);
        rfidFile.close();
        return false;
    }

    bool recordFound = false;
    while (rfidFile.available()) {
        String line = rfidFile.readStringUntil('\n');
        line.trim();

        if (line == inputRfid) {
            esplogI(" (libHash): Found RFID record: %s. Deleting...\n", inputRfid.c_str());
            recordFound = true;
        } else {
            tempFile.println(line);
        }
    }

    rfidFile.close();
    tempFile.close();

    if (!recordFound) {
        esplogW(" (libHash): RFID record not found!\n");
        SD.remove(RFID_TMP_FILE);
        return false;
    }

    if (!SD.remove(RFID_FILE)) {
        esplogE(" (libHash): Failed to delete the original RFID file!\n");
        return false;
    }

    if (!SD.rename(RFID_TMP_FILE, RFID_FILE)) {
        esplogE(" (libHash): Failed to rename temporary file to RFID file!\n");
        return false;
    }

    esplogI(" (libHash): Successfully deleted RFID record and updated the file.\n");
    return true;
}

bool existsRfid() {
    esplogI(" (libHash): Checking if rfid file exists... %d\n", SD.exists(RFID_FILE));
    return SD.exists(RFID_FILE);
}

bool checkRfid(String &inputRfid) {
    esplogI(" (libHash): Checking rfid...\n");
    inputRfid.trim();
    if (!SD.exists(RFID_FILE)) {
        esplogW(" (libHash): RFID file not found!\n");
        return false;
    }

    File rfidFile = SD.open(RFID_FILE, "r");
    if (!rfidFile) {
        esplogE(" (libHash): Failed to open RFID file: %s! Unexpected error!\n", RFID_FILE);
        return false;
    }

    bool recordFound = false;
    while (rfidFile.available()) {
        String line = rfidFile.readStringUntil('\n');
        line.trim();
        
        if (line == inputRfid) {
            esplogI(" (libHash): Found matching RFID record: %s\n", inputRfid.c_str());
            recordFound = true;
            break;
        }
    }

    rfidFile.close();

    if (recordFound) {
        esplogI(" (libHash): RFID record exists.\n");
        return true;
    } else {
        esplogW(" (libHash): RFID record not found.\n");
        return false;
    }
}
