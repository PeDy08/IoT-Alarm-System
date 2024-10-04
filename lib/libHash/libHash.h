/**
 * @file libHash.h
 * @brief Contains functions and definitions for managing alarm passwords.
 * 
 * Contains functions and definitions for managing alarm passwords.
 */

#ifndef LIBHASH_H_DEFINITION
#define LIBHASH_H_DEFINITION

#include <Arduino.h>
#include <LittleFS.h>
#include <mbedtls/sha256.h>

#include "utils.h"

#define LOCK_FILE_NAME "passwords.txt"
#define LOCK_FILE_PATH "/"
#define LOCK_FILE String(String(LOCK_FILE_PATH)+String(LOCK_FILE_NAME)).c_str()

#define MIN_PASSWORD_LENGTH 4
#define MAX_PASSWORD_LENGTH 8

/**
 * @brief Hashes a given password using SHA-256.
 * 
 * This function takes a plaintext password as input and returns its 
 * SHA-256 hash as a hexadecimal string.
 * 
 * @param password The plaintext password to hash.
 * @return The hashed password in hexadecimal format.
 */
String hashPassword(const String &password);

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
bool savePassword(const String &inputPassword);

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
bool saveNewPassword(const String &inputDoublePassword);

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
bool checkPassword(const String &inputPassword);

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

#endif