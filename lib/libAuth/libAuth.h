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

#ifdef EINK
#include "libDisplayEINK.h"
#endif

#ifdef LCD
#include "libDisplayLCD.h"
extern LiquidCrystal_I2C display;
#endif

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

// **************************************************************** PSWD ****************************************************************

/**
 * @brief Hashes the input password using SHA-256 algorithm.
 *
 * This function takes a password, trims any leading or trailing whitespace, 
 * and hashes it using the SHA-256 algorithm. The resulting hash is then 
 * converted into a hexadecimal string and returned.
 *
 * @param inputPassword The password string to be hashed.
 *        The input password is trimmed before hashing.
 *
 * @return String The hashed password as a hexadecimal string.
 *         The result is a 64-character string (32 bytes) representing the 
 *         SHA-256 hash in hexadecimal form.
 *
 * @details
 * The function uses the mbedtls library to perform the SHA-256 hashing. 
 * It initializes the mbedtls_sha256_context, updates the context with the 
 * password data, finishes the hashing process, and finally converts the 
 * resulting 32-byte hash into a string of hexadecimal characters.
 *
 * @code
 * String password = "mySecurePassword";
 * String hashedPassword = hashPassword(password);
 * Serial.println(hashedPassword); // Prints the hashed password in hexadecimal
 * @endcode
 */
String hashPassword(String &password);

/**
 * @brief Checks if the input password matches the stored password.
 *
 * This function reads a password file from the SD card, compares the input 
 * password with the stored password, and returns whether the passwords match.
 * It also handles reading the stored password's length and the corresponding 
 * hashed password to verify the input.
 *
 * @param inputPassword The password entered by the user to check against the stored password.
 *        The input password is trimmed before checking.
 *
 * @return bool Returns `true` if the input password is correct, `false` otherwise.
 *
 * @details
 * The function first checks if the password file exists on the SD card. If it doesn't,
 * an error is displayed. The function then opens the password file and reads the stored 
 * password length, stored password, and the stored hashed password. 
 * The input password is compared with the stored hash after trimming any extra characters 
 * such as the trailing `#` symbol. If the hashed input password matches the stored hash, 
 * the function returns `true`, indicating the password is correct; otherwise, it returns `false`.
 *
 * @code
 * String userPassword = "userEnteredPassword#";
 * if (checkPassword(userPassword)) {
 *     // Password is correct
 * } else {
 *     // Password is incorrect
 * }
 * @endcode
 */
bool checkPassword(String &inputPassword);

/**
 * @brief Saves the input password to the SD card after hashing.
 *
 * This function checks if the password file exists, and if so, deletes it before 
 * writing a new password. It validates the input password's length, hashes it, 
 * and stores the password's length, the password itself, and the hashed password 
 * in the password file.
 *
 * @param inputPassword The password entered by the user to be saved.
 *        The input password is trimmed before saving and checked for length validity.
 *
 * @return bool Returns `true` if the password was successfully saved, `false` otherwise.
 *
 * @details
 * The function first trims any extra spaces from the input password and checks 
 * if the password file already exists on the SD card. If the file exists, it is removed 
 * before saving the new password. The password is then validated to ensure it meets 
 * the required length constraints (minimum and maximum lengths). If the password length 
 * is valid, it is hashed using the `hashPassword` function, and both the password length, 
 * the password itself, and the hashed password are saved to the file. If the process succeeds, 
 * a success notification is shown; otherwise, an error notification is displayed.
 *
 * @code
 * String userPassword = "newPassword#";
 * if (savePassword(userPassword)) {
 *     // Password was saved successfully
 * } else {
 *     // Password saving failed
 * }
 * @endcode
 */
bool savePassword(String &inputPassword);

/**
 * @brief Saves a new password to the SD card after validation and hashing.
 *
 * This function parses the input password, checks that the two provided passwords match, 
 * validates their length, and hashes the password before saving it to the SD card.
 * If the password file exists, it will be overwritten with the new password.
 *
 * @param inputDoublePassword A string containing two passwords separated by a `#` symbol. 
 *        The function extracts the first password, checks if it matches the second password, 
 *        and saves the validated password to the SD card.
 *
 * @return bool Returns `true` if the password was successfully saved, `false` otherwise.
 *
 * @details
 * The function first trims the input password and checks if the password file exists on 
 * the SD card. If it exists, the file is deleted and overwritten. The input password is 
 * expected to contain two passwords separated by the `#` symbol. If the two passwords don't 
 * match, the function returns `false`. If the password length is invalid (too short or too long), 
 * an error is returned. The valid password is then hashed and saved along with its length 
 * to the password file on the SD card. 
 *
 * @code
 * String userDoublePassword = "newPassword#newPassword";
 * if (saveNewPassword(userDoublePassword)) {
 *     // New password was saved successfully
 * } else {
 *     // New password saving failed
 * }
 * @endcode
 */
bool saveNewPassword(String &inputDoublePassword);

/**
 * @brief Checks if the password file exists on the SD card.
 *
 * This function checks whether the password file exists on the SD card. It returns `true` 
 * if the file exists, otherwise, it returns `false`.
 *
 * @param None
 *
 * @return bool Returns `true` if the password file exists, `false` otherwise.
 *
 * @details
 * The function checks for the existence of a specific file, identified by the `LOCK_FILE` 
 * constant, on the SD card. The existence check is performed using the `SD.exists()` method, 
 * which checks if the specified file path is valid and available. The function logs the 
 * check result and returns a boolean indicating whether the password file is present.
 *
 * @code
 * if (existsPassword()) {
 *     // Password file exists
 * } else {
 *     // Password file does not exist
 * }
 * @endcode
 */
bool existsPassword();

/**
 * @brief Computes the length of the stored password from the SD card.
 *
 * This function reads the stored password length from the password file on the SD card. 
 * It returns the length of the password if the file exists and can be opened, otherwise, 
 * it returns `0` if the file doesn't exist or `-1` if there is an error opening the file.
 *
 * @param None
 *
 * @return int Returns the stored password length, `0` if the file doesn't exist, 
 *         and `-1` if there is an error opening the file.
 *
 * @details
 * The function first checks if the password file exists on the SD card using the `SD.exists()` method. 
 * If the file exists, it opens the file and reads the first line (the stored password length) using 
 * `readStringUntil('\n')`. The length is then returned as an integer. If the file doesn't exist or 
 * there is an error reading it, the function logs the issue and returns an appropriate value: `0` 
 * if the file doesn't exist and `-1` if the file cannot be opened.
 *
 * @code
 * int passwordLength = lengthPassword();
 * if (passwordLength > 0) {
 *     // The password length is valid
 * } else if (passwordLength == 0) {
 *     // Password file does not exist
 * } else {
 *     // Error opening the password file
 * }
 * @endcode
 */
int lengthPassword();

// **************************************************************** RFID ****************************************************************

/**
 * @brief Checks if the specified RFID exists in the RFID file.
 *
 * This function checks if the provided RFID is found in the RFID file stored on the SD card.
 * It reads through the file, searching for a match to the input RFID. If a match is found,
 * it returns `true` and logs the success. If no match is found, it returns `false` and logs 
 * the error.
 *
 * @param inputRfid The RFID to check against the records in the RFID file.
 * @return bool Returns `true` if the input RFID is found in the RFID file, `false` otherwise.
 *
 * @details
 * The function opens the RFID file and reads each line to check if the input RFID matches 
 * any existing record in the file. If the RFID is found, the function returns `true`. 
 * If not, the function returns `false`.
 *
 * @code
 * if (checkRfid(rfid)) {
 *     // RFID exists in the file
 * } else {
 *     // RFID does not exist
 * }
 * @endcode
 */
bool checkRfid(String &inputRfid);

/**
 * @brief Saves the RFID UID to a file on the SD card.
 *
 * This function saves the provided RFID UID to a file on the SD card. If the RFID file already 
 * exists, it will be overwritten. If the file cannot be opened or the existing file cannot be 
 * removed, the function returns `false`; otherwise, it returns `true` after successfully saving 
 * the RFID UID.
 *
 * @param inputRfid The RFID UID to be saved to the file.
 * 
 * @return bool Returns `true` if the RFID UID was successfully saved, `false` if there was an error 
 *         opening the file or rewriting the existing file.
 *
 * @details
 * The function first checks if the RFID file already exists on the SD card using `SD.exists()`. 
 * If the file exists, it attempts to remove it using `SD.remove()`. If the file is successfully 
 * removed, or if the file doesn't exist, it opens the file for writing. The RFID UID is then written 
 * to the file using `rfidFile.println()`, and the file is closed. If any issues arise during file 
 * handling (such as failure to open or remove the file), the function logs an error and returns `false`.
 *
 * @code
 * if (saveRfid(rfid)) {
 *     // RFID UID saved successfully
 * } else {
 *     // Error saving RFID UID
 * }
 * @endcode
 */
bool saveRfid(String &inputRfid);

/**
 * @brief Adds a new RFID record to the RFID file on the SD card.
 *
 * This function checks if the provided RFID UID already exists in the RFID file. If it does not, 
 * the function appends the new RFID UID to the file. If the file doesn't exist, it creates a new file 
 * and saves the UID. If the UID already exists, the function logs a message and returns `true` without 
 * adding the UID again.
 *
 * @param inputRfid The RFID UID to be added to the file.
 * 
 * @return bool Returns `true` if the RFID UID was successfully added or if it already exists, 
 *         `false` if there was an error opening or appending the file.
 *
 * @details
 * The function first checks if the RFID file exists by opening it in read mode (`r`). It then reads 
 * through the file line by line, comparing each line with the provided RFID UID. If a match is found, 
 * the function logs that the RFID is already present and returns `true`. If no match is found, the function 
 * opens the file in append mode (`a`) or creates a new file if it doesn't exist, and appends the RFID UID 
 * to the file. Any errors in file handling result in an error message and the function returning `false`.
 *
 * @code
 * if (addRfid(rfid)) {
 *     // RFID UID successfully added or already exists
 * } else {
 *     // Error adding RFID UID
 * }
 * @endcode
 */
bool addRfid(String &inputRfid);

/**
 * @brief Deletes a specific RFID record from the RFID file.
 *
 * This function deletes the provided RFID UID from the RFID file on the SD card. It first searches 
 * for the record in the file, and if found, removes it by creating a temporary file, copying all 
 * non-matching records into it, and then renaming the temporary file to replace the original file.
 * If the RFID record is not found, it returns `false`. If there are issues opening or manipulating 
 * files, it will return `false` as well.
 *
 * @param inputRfid The RFID UID to be deleted from the file.
 * 
 * @return bool Returns `true` if the RFID record was successfully deleted and the file updated, 
 *         `false` if there was an error (e.g., file not found, record not found, or failure to 
 *         manipulate files).
 *
 * @details
 * The function first checks if the RFID file exists on the SD card. If it does, the function opens 
 * it for reading and then creates a temporary file for storing the remaining RFID records. It reads 
 * through the original file line by line, copying records that do not match the `inputRfid` to the 
 * temporary file. If a matching record is found, it is not written to the temporary file. After the 
 * file processing, the original RFID file is deleted, and the temporary file is renamed to replace 
 * the original. If the record was not found or any file operation fails, the function returns `false`.
 *
 * @code
 * if (delRfid(rfid)) {
 *     // RFID UID deleted successfully
 * } else {
 *     // Error deleting RFID UID
 * }
 * @endcode
 */
bool delRfid(String &inputRfid);

/**
 * @brief Checks if the RFID file exists on the SD card.
 *
 * This function checks whether the RFID file exists on the SD card. It returns `true` if the 
 * file is found, and `false` if the file does not exist or there is an error accessing the 
 * SD card.
 *
 * @return bool Returns `true` if the RFID file exists, `false` otherwise.
 *
 * @details
 * The function uses the `SD.exists()` method to check if the file specified by `RFID_FILE` 
 * exists on the SD card. It logs the result of the check for debugging purposes.
 *
 * @code
 * if (existsRfid()) {
 *     // RFID file exists
 * } else {
 *     // RFID file does not exist
 * }
 * @endcode
 */
bool existsRfid();

#endif