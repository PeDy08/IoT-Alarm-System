#include "libAuth.h"

MFRC522 mfrc522(RFID_CS_PIN, RFID_RST_PIN);

String hashPassword(String &inputPassword) {
    esplogI(TAG_LIB_AUTH, "(hashPassword)", "Hashing password...");
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
    esplogI(TAG_LIB_AUTH, "(checkPassword)", "Checking password...");
    inputPassword.trim();
    if (!SD.exists(LOCK_FILE)) {
        esplogW(TAG_LIB_AUTH, "(checkPassword)", "Password file doesn't exist!");
        displayNotification(NOTIFICATION_AUTH_CHECK_ERROR);
        return false;
    }

    File passwordFile = SD.open(LOCK_FILE, "r");
    if (!passwordFile) {
        esplogE(TAG_LIB_AUTH, "(checkPassword)", "Failed to open password file: '%s'! Unexpected error!", LOCK_FILE);
        displayNotification(NOTIFICATION_AUTH_CHECK_ERROR);
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
        esplogI(TAG_LIB_AUTH, "(checkPassword)", "Incorrect password! Password length mismatch!");
        passwordFile.close();
        displayNotification(NOTIFICATION_AUTH_CHECK_ERROR);
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
        esplogI(TAG_LIB_AUTH, "(checkPassword)", "Password is correct.");
        displayNotification(NOTIFICATION_AUTH_CHECK_SUCCESS);
        return true;
    } else {
        esplogI(TAG_LIB_AUTH, "(checkPassword)", "Incorrect password!");
        displayNotification(NOTIFICATION_AUTH_CHECK_ERROR);
        return false;
    }
}

bool savePassword(String &inputPassword) {
    esplogI(TAG_LIB_AUTH, "(savePassword)", "Saving password to file...");
    inputPassword.trim();
    if (SD.exists(LOCK_FILE)) {
        esplogW(TAG_LIB_AUTH, "(savePassword)", "Password file found, rewriting!");
        if (!SD.remove(LOCK_FILE)) {
            esplogE(TAG_LIB_AUTH, "(savePassword)", "Failed to rewrite existing file: %s!", LOCK_FILE);
            displayNotification(NOTIFICATION_AUTH_SET_ERROR);
            return false;
        }
    }

    File passwordFile = SD.open(LOCK_FILE, "w");
    if (!passwordFile) {
        esplogE(TAG_LIB_AUTH, "(savePassword)", "Failed to open password file: %s when writing! Unexpected error!", LOCK_FILE);
        displayNotification(NOTIFICATION_AUTH_SET_ERROR);
        return false;
    }

    String actualPassword;
    if (inputPassword.endsWith("#")) {
        actualPassword = inputPassword.substring(0, inputPassword.length() - 1);
    } else {
        actualPassword = inputPassword;
    }

    if (actualPassword.length() < MIN_PASSWORD_LENGTH || actualPassword.length() > MAX_PASSWORD_LENGTH) {
        esplogW(TAG_LIB_AUTH, "(savePassword)", "Failed to save new password! Password was too short or too long!");
        passwordFile.close();
        displayNotification(NOTIFICATION_AUTH_SET_ERROR);
        return false;
    }

    String hashedPassword = hashPassword(actualPassword);
    passwordFile.println(actualPassword.length());
    passwordFile.println(actualPassword);
    passwordFile.println(hashedPassword);
    passwordFile.close();
    displayNotification(NOTIFICATION_AUTH_SET_SUCCESS);
    return true;
}

bool saveNewPassword(String &inputDoublePassword) {
    esplogI(TAG_LIB_AUTH, "(saveNewPassword)", "Saving new password to file...");
    inputDoublePassword.trim();
    if (SD.exists(LOCK_FILE)) {
        esplogW(TAG_LIB_AUTH, "(saveNewPassword)", "Password file found, rewriting!");
        if (!SD.remove(LOCK_FILE)) {
            esplogE(TAG_LIB_AUTH, "(saveNewPassword)", "Failed to rewrite existing file: %s!", LOCK_FILE);
            displayNotification(NOTIFICATION_AUTH_SET_ERROR);
            return false;
        }
    }

    int firstDelimiterPos = inputDoublePassword.indexOf('#');
    int secondDelimiterPos = inputDoublePassword.lastIndexOf('#');

    if (firstDelimiterPos == -1 || secondDelimiterPos == -1 || firstDelimiterPos == secondDelimiterPos) {
        esplogW(TAG_LIB_AUTH, "(saveNewPassword)", "Failed to save new password! Password string parsing error! Unexpected string format: %s", inputDoublePassword);
        displayNotification(NOTIFICATION_AUTH_SET_ERROR);
        return false;
    }

    String firstPassword = inputDoublePassword.substring(0, firstDelimiterPos);
    String secondPassword = inputDoublePassword.substring(firstDelimiterPos+1, secondDelimiterPos);
    
    if (firstPassword != secondPassword) {
        esplogW(TAG_LIB_AUTH, "(saveNewPassword)", "Failed to save new password! Different passwords were written on setup!\n - first:  %s\n - second: %s", firstPassword, secondPassword);
        displayNotification(NOTIFICATION_AUTH_SET_ERROR);
        return false;
    }

    if (firstPassword.length() < MIN_PASSWORD_LENGTH || firstPassword.length() > MAX_PASSWORD_LENGTH) {
        esplogW(TAG_LIB_AUTH, "(saveNewPassword)", "Failed to save new password! Password was too short or too long!");
        displayNotification(NOTIFICATION_AUTH_SET_ERROR);
        return false;
    }

    File passwordFile = SD.open(LOCK_FILE, "w");
    if (!passwordFile) {
        esplogE(TAG_LIB_AUTH, "(saveNewPassword)", "Failed to open password file: %s when writing! Unexpected error!", LOCK_FILE);
        displayNotification(NOTIFICATION_AUTH_SET_ERROR);
        return false;
    }

    String hashedPassword = hashPassword(firstPassword);
    passwordFile.println(firstPassword.length());
    passwordFile.println(firstPassword);
    passwordFile.println(hashedPassword);
    passwordFile.close();
    displayNotification(NOTIFICATION_AUTH_SET_SUCCESS);
    return true;
}

bool existsPassword() {
    esplogI(TAG_LIB_AUTH, "(existsPassword)", "Checking if password exists... %d", SD.exists(LOCK_FILE));
    return SD.exists(LOCK_FILE);
}

int lengthPassword() {
    esplogI(TAG_LIB_AUTH, "(lengthPassword)", "Computing password length...");
    if (!SD.exists(LOCK_FILE)) {
        esplogW(TAG_LIB_AUTH, "(lengthPassword)", "Password file doesn't exist!");
        return 0;
    }

    File passwordFile = SD.open(LOCK_FILE, "r");
    if (!passwordFile) {
        esplogE(TAG_LIB_AUTH, "(lengthPassword)", "Failed to open password file: '%s'! Unexpected error!", LOCK_FILE);
        return -1;
    }

    int storedPasswordLength = passwordFile.readStringUntil('\n').toInt();
    return storedPasswordLength;
}


// ******************************** RFID ********************************
bool saveRfid(String &inputRfid) {
    esplogI(TAG_LIB_AUTH, "(saveRfid)", "Saving rfid to file...");
    inputRfid.trim();
    if (SD.exists(RFID_FILE)) {
        esplogW(TAG_LIB_AUTH, "(saveRfid)", "RFID file found, rewriting!");
        if (!SD.remove(RFID_FILE)) {
            esplogE(TAG_LIB_AUTH, "(saveRfid)", "Failed to rewrite existing file: %s!", RFID_FILE);
            return false;
        }
    }

    File rfidFile = SD.open(RFID_FILE, "w");
    if (!rfidFile) {
        esplogE(TAG_LIB_AUTH, "(saveRfid)", "Failed to open RFID file: %s! Unexpected error!", RFID_FILE);
        return false;
    }

    // if is needed any check for input RFID UID here needs to be implemented

    rfidFile.println(inputRfid);
    rfidFile.close();
    return true;
}

bool addRfid(String &inputRfid) {
    esplogI(TAG_LIB_AUTH, "(addRfid)", "Adding new rfid record to RFID file...");
    inputRfid.trim();

    File rfidFile = SD.open(RFID_FILE, "r");
    bool recordFound = false;
    while (rfidFile.available()) {
        String line = rfidFile.readStringUntil('\n');
        line.trim();
        
        if (line == inputRfid) {
            esplogI(TAG_LIB_AUTH, "(addRfid)", "Found matching RFID record: %s", inputRfid.c_str());
            recordFound = true;
            break;
        }
    }

    rfidFile.close();
    if (recordFound) {
        esplogW(TAG_LIB_AUTH, "(addRfid)", "RFID UID already added: %s! Ignoring...", inputRfid);
        displayNotification(NOTIFICATION_RFID_ADD_SUCCESS);
        return true;
    }

    if (!SD.exists(RFID_FILE)) {
        esplogW(TAG_LIB_AUTH, "(addRfid)", "RFID file not found! Creating new file!");
        rfidFile = SD.open(RFID_FILE, "w");
    } else {
        esplogW(TAG_LIB_AUTH, "(addRfid)", "RFID file found! Appending record to existing file!");
        rfidFile = SD.open(RFID_FILE, "a");
    }

    if (!rfidFile) {
        esplogE(TAG_LIB_AUTH, "(addRfid)", "Failed to open RFID file: %s! Unexpected error!", RFID_FILE);
        displayNotification(NOTIFICATION_RFID_ADD_ERROR);
        return false;
    }

    // if is needed any check for input RFID UID here needs to be implemented

    rfidFile.println(inputRfid);
    rfidFile.close();
    displayNotification(NOTIFICATION_RFID_ADD_SUCCESS);
    return true;
}

bool delRfid(String &inputRfid) {
    esplogI(TAG_LIB_AUTH, "(delRfid)", "Deleting rfid record from file...");
    inputRfid.trim();
    if (!SD.exists(RFID_FILE)) {
        esplogW(TAG_LIB_AUTH, "(delRfid)", "RFID file not found!");
        displayNotification(NOTIFICATION_RFID_DEL_ERROR);
        return false;
    }

    File rfidFile = SD.open(RFID_FILE, "r");
    if (!rfidFile) {
        esplogE(TAG_LIB_AUTH, "(delRfid)", "Failed to open RFID file: %s! Unexpected error!", RFID_FILE);
        displayNotification(NOTIFICATION_RFID_DEL_ERROR);
        return false;
    }

    File tempFile = SD.open(RFID_TMP_FILE, "w");
    if (!tempFile) {
        esplogE(TAG_LIB_AUTH, "(delRfid)", "Failed to create temporary file: %s! Unexpected error!", RFID_TMP_FILE);
        rfidFile.close();
        displayNotification(NOTIFICATION_RFID_DEL_ERROR);
        return false;
    }

    bool recordFound = false;
    while (rfidFile.available()) {
        String line = rfidFile.readStringUntil('\n');
        line.trim();

        if (line == inputRfid) {
            esplogI(TAG_LIB_AUTH, "(delRfid)", "Found RFID record: %s. Deleting...", inputRfid.c_str());
            recordFound = true;
        } else {
            tempFile.println(line);
        }
    }

    rfidFile.close();
    tempFile.close();

    if (!recordFound) {
        esplogW(TAG_LIB_AUTH, "(delRfid)", "RFID record not found!");
        SD.remove(RFID_TMP_FILE);
        displayNotification(NOTIFICATION_RFID_DEL_ERROR);
        return false;
    }

    if (!SD.remove(RFID_FILE)) {
        esplogE(TAG_LIB_AUTH, "(delRfid)", "Failed to delete the original RFID file!");
        displayNotification(NOTIFICATION_RFID_DEL_ERROR);
        return false;
    }

    if (!SD.rename(RFID_TMP_FILE, RFID_FILE)) {
        esplogE(TAG_LIB_AUTH, "(delRfid)", "Failed to rename temporary file to RFID file!");
        displayNotification(NOTIFICATION_RFID_DEL_ERROR);
        return false;
    }

    esplogI(TAG_LIB_AUTH, "(delRfid)", "Successfully deleted RFID record and updated the file.");
    displayNotification(NOTIFICATION_RFID_DEL_SUCCESS);
    return true;
}

bool existsRfid() {
    esplogI(TAG_LIB_AUTH, "(existsRfid)", "Checking if rfid file exists... %d", SD.exists(RFID_FILE));
    return SD.exists(RFID_FILE);
}

bool checkRfid(String &inputRfid) {
    esplogI(TAG_LIB_AUTH, "(checkRfid)", "Checking rfid...");
    inputRfid.trim();
    if (!SD.exists(RFID_FILE)) {
        esplogW(TAG_LIB_AUTH, "(checkRfid)", "RFID file not found!");
        displayNotification(NOTIFICATION_RFID_CHECK_ERROR);
        return false;
    }

    File rfidFile = SD.open(RFID_FILE, "r");
    if (!rfidFile) {
        esplogE(TAG_LIB_AUTH, "(checkRfid)", "Failed to open RFID file: %s! Unexpected error!", RFID_FILE);
        displayNotification(NOTIFICATION_RFID_CHECK_ERROR);
        return false;
    }

    bool recordFound = false;
    while (rfidFile.available()) {
        String line = rfidFile.readStringUntil('\n');
        line.trim();
        
        if (line == inputRfid) {
            esplogI(TAG_LIB_AUTH, "(checkRfid)", "Found matching RFID record: %s", inputRfid.c_str());
            recordFound = true;
            break;
        }
    }

    rfidFile.close();

    if (recordFound) {
        esplogI(TAG_LIB_AUTH, "(checkRfid)", "RFID record exists.");
        displayNotification(NOTIFICATION_RFID_CHECK_SUCCESS);
        return true;
    } else {
        esplogW(TAG_LIB_AUTH, "(checkRfid)", "RFID record not found.");
        displayNotification(NOTIFICATION_RFID_CHECK_ERROR);
        return false;
    }
}
