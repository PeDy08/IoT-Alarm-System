#include "libAuth.h"

MFRC522 mfrc522(RFID_CS_PIN);

String hashPassword(const String &password) {
    esplogI(" (libHash): Hashing password...\n");
    uint8_t hash[32]; // SHA-256 produces a 32-byte hash
    mbedtls_sha256_context ctx;
    
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0); // 0 for SHA-256
    mbedtls_sha256_update(&ctx, (const unsigned char *)password.c_str(), password.length());
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);

    // Convert hash to a hexadecimal string
    String hashedPassword;
    for (int i = 0; i < 32; i++) {
        hashedPassword += String(hash[i], HEX);
    }
    return hashedPassword;
}

bool checkPassword(const String &inputPassword) {
    esplogI(" (libHash): Checking password...\n");
    if (!LittleFS.exists(LOCK_FILE)) {
        esplogW(" (libHash): Password file doesn't exist!\n");
        return false;
    }

    File passwordFile = LittleFS.open(LOCK_FILE, "r");
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

    esplogI("L:%d\nI:%s\nS:%s\nH:%s\n", actualPassword.length(), actualPassword, storedPassword, hashedPassword);
    passwordFile.close();
    
    if (hashedPassword == storedHashedPassword) {
        esplogI(" (libHash): Password is correct.\n");
        return true;
    } else {
        esplogI(" (libHash): Incorrect password!\n");
        return false;
    }
}

bool savePassword(const String &inputPassword) {
    esplogI(" (libHash): Saving password to file...\n");
    if (LittleFS.exists(LOCK_FILE)) {
        esplogW(" (libHash): Password file found, rewriting!\n");
        if (!LittleFS.remove(LOCK_FILE)) {
            esplogE(" (libHash): Failed to rewrite existing file: %s!\n", LOCK_FILE);
            return false;
        }
    }

    File passwordFile = LittleFS.open(LOCK_FILE, "w");
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

bool saveNewPassword(const String &inputDoublePassword) {
    esplogI(" (libHash): Saving new password to file...\n");
    if (LittleFS.exists(LOCK_FILE)) {
        esplogW(" (libHash): Password file found, rewriting!\n");
        if (!LittleFS.remove(LOCK_FILE)) {
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

    File passwordFile = LittleFS.open(LOCK_FILE, "w");
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
    esplogI(" (libHash): Checking if password exists...\n");
    return LittleFS.exists(LOCK_FILE);
}

int lengthPassword() {
    esplogI(" (libHash): Computing password length...\n");
    if (!LittleFS.exists(LOCK_FILE)) {
        esplogW(" (libHash): Password file doesn't exist!\n");
        return 0;
    }

    File passwordFile = LittleFS.open(LOCK_FILE, "r");
    if (!passwordFile) {
        esplogE(" (libJson): Failed to open password file: '%s'! Unexpected error!\n", LOCK_FILE);
        return -1;
    }

    int storedPasswordLength = passwordFile.readStringUntil('\n').toInt();
    return storedPasswordLength;
}
