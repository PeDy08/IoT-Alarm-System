/**
 * @file libAuth.h
 * @brief Contains functions and definitions for managing alarm passwords and authorisation.
 * 
 * Contains functions and definitions for managing alarm passwords. RFID cards etc...
 */

#ifndef LIBAUTH_H_DEFINITION
#define LIBAUTH_H_DEFINITION

#include <Arduino.h>
#include <SD.h>
#include <mbedtls/sha256.h>
#include <MFRC522.h>

#include "utils.h"

#define LOCK_FILE_NAME "passwords.txt"
#define LOCK_FILE_PATH "/auth/"
#define LOCK_FILE String(String(LOCK_FILE_PATH)+String(LOCK_FILE_NAME)).c_str()

#define RFID_FILE_NAME "rfids.txt"
#define RFID_FILE_PATH "/auth/"
#define RFID_FILE String(String(RFID_FILE_PATH)+String(RFID_FILE_NAME)).c_str()

#define RFID_TMP_FILE_NAME "rfids_tmp.txt"
#define RFID_TMP_FILE_PATH "/auth/"
#define RFID_TMP_FILE String(String(RFID_TMP_FILE_PATH)+String(RFID_TMP_FILE_NAME)).c_str()

#define MIN_PASSWORD_LENGTH 4
#define MAX_PASSWORD_LENGTH 8

#define RFID_CS_PIN 32
#define RFID_RST_PIN 33

extern MFRC522 mfrc522;

/**
 * @brief Hashes a given password using SHA-256.
 * 
 * This function takes a plaintext password as input and returns its 
 * SHA-256 hash as a hexadecimal string.
 * 
 * @param password The plaintext password to hash.
 * @return The hashed password in hexadecimal format.
 */
String hashPassword(String &password);

/**
 * @brief Saves the hashed password to a file on LittleFS.
 * 
 * This function opens a file on LittleFS in write mode and saves the 
 * provided hashed password.
 * If the file cannot be opened, it returns false. If given password is in wrong format, it returns false.
 * 
 * @param inputPassword The hashed password to save.
 * @return True if the password was successfully saved, false otherwise.
 */
bool savePassword(String &inputPassword);

/**
 * @brief Saves the hashed password to a file on LittleFS.
 * 
 * This function opens a file on LittleFS in write mode and saves the 
 * provided hashed password.
 * If the file cannot be opened, it returns false. If given password is in wrong format, it returns false.
 * The difference between `savePassword()` and `saveNewPassword` is that
 * this function expects full string with two times repeated password and '*' or '#' character at the end.
 * 
 * @param inputDoublePassword The hashed password to save.
 * @return True if the password was successfully saved, false otherwise.
 */
bool saveNewPassword(String &inputDoublePassword);

/**
 * @brief Checks if the entered password matches the stored hashed password.
 * 
 * This function hashes the input password and compares it with the 
 * hashed password stored in a file on LittleFS. It returns true if 
 * they match and false otherwise.
 * 
 * @param inputPassword The plaintext password entered by the user.
 * @return True if the password is correct, false otherwise.
 */
bool checkPassword(String &inputPassword);

/**
 * @brief Checks if the password was set or not.
 * 
 * @return True if the password file exists, false otherwise.
 */
bool existsPassword();

/**
 * @brief Gets the length of the stored password.
 * 
 * This function reads the first line from the password file,
 * which contains the length of the stored password, and returns it as an integer.
 * 
 * @return The length of the stored password, or -1 if there was an error or 0 if no password was set.
 */
int lengthPassword();

/**
 * @brief Checks if the detected rfid card matches the stored and authorised cards.
 * 
 * This function reads the rfid uid and compares it with the 
 * uids stored in a file on LittleFS. It returns true if 
 * they match and false otherwise.
 * 
 * @param inputRfid The plaintext rfid uid detected by the rfid reader.
 * @return True if the uids match, false otherwise.
 */
bool checkRfid(String &inputRfid);

/**
 * @brief Saves the detected rfid card uid to a file on LittleFS.
 * 
 * This function opens a file on LittleFS in write mode and saves the 
 * provided rfid uid data.
 * If the file cannot be opened, it returns false.
 * 
 * @param inputRfid The detected rfid card uid to save.
 * @return True if the uid was successfully saved, false otherwise.
 */
bool saveRfid(String &inputRfid);

/**
 * @brief Add new record of detected rfid card uid to a file on LittleFS.
 * 
 * This function opens a file on LittleFS in apppend mode and appends the 
 * provided rfid uid data.
 * If the file cannot be opened, it returns false.
 * 
 * @param inputRfid The detected rfid card uid to save.
 * @return True if the uid was successfully added, false otherwise.
 */
bool addRfid(String &inputRfid);

/**
 * @brief Delete a record of detected rfid card uid to a file on LittleFS.
 * 
 * This function opens a file on LittleFS in edit mode and deletes the 
 * provided rfid uid data.
 * If the file cannot be opened, it returns false. If record is not found, it returns false.
 * 
 * @param inputRfid The detected rfid card uid to delete.
 * @return True if the uid was successfully deleted, false otherwise.
 */
bool delRfid(String &inputRfid);

/**
 * @brief Checks if any rfid was set or not.
 * 
 * @return True if the rfid file exists, false otherwise.
 */
bool existsRfid();

#endif