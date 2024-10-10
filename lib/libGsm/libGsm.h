/**
 * @file libGsm.h
 * @brief Contains functions and definitions for comunicating with gsm module SIM800L.
 * 
 * Contains functions and definitions for comunicating with gsm module SIM800L.
 */

#ifndef LIBGSM_H_DEFINITION
#define LIBGSM_H_DEFINITION

#include <Arduino.h>
#include <HardwareSerial.h>

#include "utils.h"

#define GSM_RX_PIN 26
#define GSM_TX_PIN 27
#define GSM_RST_PIN 25
#define GSM_BAUDRATE 9600
#define GSM_TIMEOUT 1000
// #define GSM_PIN ""  // <- not implemented (command: AT+CPIN=****)

// volatile functions
// #define GSM_SLEEP
// #define GSM_PWRDN

extern HardwareSerial SerialGSM;

struct CallInfo {
    int id;          // <id1> Call identification number
    int dir;         // <dir> Direction of the call (0 = MO, 1 = MT)
    int stat;        // <stat> State of the call (0 = Active, 1 = Held, 2 = Dialing, 3 = Alerting, etc.)
    int mode;        // <mode> Bearer/tele service (0 = Voice, 1 = Data, 2 = Fax)
    int mpty;        // <mpty> Multiparty call (0 = No, 1 = Yes)
    String number;   // <number> Phone number (optional)
    int type;        // <type> Type of address (optional)
    String alphaID;  // <alphaID> Alphanumeric representation (optional)
};

struct SmsInfo {
    int index;         // Index of SMS in storage
    String status;     // Status of SMS in storage
    String origin;     // Sender's phone number
    String datetime;   // Date and time of the message
    String message;    // The actual message content
};

inline CallInfo parseCLCCResponse(String response) {
    CallInfo call;

    // Check if the response contains "+CLCC:", which means there was a call detected
    int cmglPos = response.indexOf("+CLCC:");
    if (cmglPos < 0) {
        // No call found, return default empty CallInfo
        call.id = -1;        
        call.dir = -1;       
        call.stat = -1;      
        call.mode = -1;      
        call.mpty = -1;      
        call.number = ""; 
        call.type = -1;      
        call.alphaID = "";
        return call;
    }

    // Extract id
    int idStart = response.indexOf("+CLCC: ") + 7;  // After '+CLCC: '
    int idEnd = response.indexOf(",", idStart);
    call.id = response.substring(idStart, idEnd).toInt();

    // Extract dir
    int dirStart = idEnd + 1;
    int dirEnd = response.indexOf(",", dirStart);
    call.dir = response.substring(dirStart, dirEnd).toInt();

    // Extract stat
    int statStart = dirEnd + 1;
    int statEnd = response.indexOf(",", statStart);
    call.stat = response.substring(statStart, statEnd).toInt();

    // Extract mode
    int modeStart = statEnd + 1;
    int modeEnd = response.indexOf(",", modeStart);
    call.mode = response.substring(modeStart, modeEnd).toInt();

    // Extract mpty
    int mptyStart = modeEnd + 1;
    int mptyEnd = response.indexOf(",", mptyStart);
    call.mpty = response.substring(mptyStart, mptyEnd).toInt();

    // Extract number (optional, might not exist)
    uint8_t numStart = mptyEnd + 1;
    if (numStart < response.length() && response[numStart] == '"') {  // Check if number is present
        int numEnd = response.indexOf('"', numStart + 1);
        call.number = response.substring(numStart + 1, numEnd);  // Extract number inside quotes
        numStart = numEnd + 2;  // Move past the number and comma
    } else {
        call.number = "";  // No number provided
    }

    // Extract type (optional, might not exist)
    if (numStart < response.length() && response[numStart] != '\r') {  // Check if type exists
        int typeEnd = response.indexOf(",", numStart);
        call.type = response.substring(numStart, typeEnd).toInt();
        numStart = typeEnd + 1;
    } else {
        call.type = -1;  // No type provided
    }

    // Extract alphaID (optional, might not exist)
    if (numStart < response.length() && response[numStart] == '"') {  // Check if alphaID exists
        int alphaEnd = response.indexOf('"', numStart + 1);
        call.alphaID = response.substring(numStart + 1, alphaEnd);  // Extract alphaID inside quotes
    } else {
        call.alphaID = "";  // No alphaID provided
    }

    return call;
}

inline SmsInfo parseCMGLResponse(String response) {
    SmsInfo sms;
    
    // Check if the response contains "+CMGL:", which means there is an SMS
    int cmglPos = response.indexOf("+CMGL:");
    if (cmglPos < 0) {
        // No SMS found, return default empty SmsInfo
        sms.index = -1;
        sms.status = "";
        sms.origin = "";
        sms.datetime = "";
        sms.message = "";
        return sms;
    }

    // Extract the SMS index
    int indexStart = cmglPos + 7;  // Position after "+CMGL: "
    int indexEnd = response.indexOf(",", indexStart);
    sms.index = response.substring(indexStart, indexEnd).toInt();

    // Extract the SMS status
    int statusStart = response.indexOf("\"", indexEnd + 1) + 1;
    int statusEnd = response.indexOf("\"", statusStart);
    sms.status = response.substring(statusStart, statusEnd);

    // Extract the origin phone number
    int originStart = response.indexOf("\"", statusEnd + 1) + 1;
    int originEnd = response.indexOf("\"", originStart);
    sms.origin = response.substring(originStart, originEnd);

    // Extract the datetime
    int dateTimeStart = response.indexOf("\"", originEnd + 1) + 1;
    int dateTimeEnd = response.indexOf("\"", dateTimeStart);
    dateTimeStart = response.indexOf("\"", dateTimeEnd + 1) + 1;
    dateTimeEnd = response.indexOf("\"", dateTimeStart);
    sms.datetime = response.substring(dateTimeStart, dateTimeEnd);

    // Extract the message content
    int messageStart = response.indexOf("\n", dateTimeEnd) + 1;
    int messageEnd = response.indexOf("\nOK", messageStart);
    sms.message = response.substring(messageStart, messageEnd);
    sms.message.trim(); // Use trim to remove any extra newline

    return sms;
}

/**
 * @brief Updates the communication between the Arduino and the GSM module.
 * 
 * This function checks if there is any data available on the Serial port and sends it
 * to the GSM module. Similarly, it reads any available data from the GSM module and 
 * sends it to the Serial port for debugging purposes.
 */
void updateSerialGSM();

/**
 * @brief Initializes the GSM module by sending several AT commands and validating the responses.
 * 
 * This function sets up the GSM module by sending a series of AT commands to verify
 * its readiness and SIM card status. It also checks the signal quality and registration status
 * of the SIM card. If any of the steps fail, it returns false.
 * 
 * @return true if the GSM module was successfully initialized, false otherwise.
 */
bool initSerialGSM();

#ifdef GSM_SLEEP
/**
 * @brief Puts the GSM module into sleep mode or wakes it up.
 *
 * This function controls the sleep state of the GSM module using AT commands.
 * 
 * If the `sleep` parameter is true, the function sends the command `AT+CSCLK=1`
 * to put the GSM module into sleep mode. It waits for a response indicating 
 * the command was executed successfully. If `sleep` is false, it sends the 
 * command `AT+CSCLK=0` to wake the module up from sleep.
 * 
 * @param sleep A boolean value indicating the desired sleep state of the 
 *              GSM module. If true, the function will attempt to put the 
 *              module into sleep mode; if false, it will attempt to wake it up.
 * 
 * @return true if the operation was successful (the module entered or exited 
 *         sleep mode), false if the operation failed (e.g., if the GSM module 
 *         did not respond appropriately).
 */
bool sleepSerialGSM(bool sleep);
#endif

#ifdef GSM_PWRDN
/**
 * @brief Controls the power state of the GSM module.
 *
 * This function powers off or powers on the GSM module using AT commands.
 * 
 * When powering off, the function sends the command `AT+CPOWD=1` to gracefully
 * shut down the module. It waits for the response indicating the power down 
 * is normal. When powering on, it sends the `AT` command to check if the 
 * module is responsive after a power down.
 * 
 * @param powerOn A boolean value that determines the power state of the 
 *                GSM module. If true, the function will attempt to power 
 *                on the module; if false, it will attempt to power it off.
 * 
 * @return true if the power state change was successful, false if the 
 *         operation failed (e.g., if the GSM module did not respond 
 *         appropriately).
 */
bool powerControlSerialGSM(bool power);
#endif

#ifdef GSM_RST_PIN
/**
 * @brief Resets the GSM module by toggling the reset pin.
 *
 * This function performs a hardware reset of the GSM module by pulling the
 * reset pin low for a brief period and then releasing it. The GSM serial 
 * connection is temporarily terminated during the reset process, and 
 * reinitialized afterward.
 *
 * @return true if the reset was successful, false otherwise.
 */
bool resetSerialGSM();
#endif

/**
 * @brief Sends an SMS message through the GSM module.
 * 
 * Sends an AT command to send an SMS to a specified phone number with a provided text message.
 * 
 * @param phoneNumber The phone number to send the SMS to.
 * @param message The text message to send.
 * @return true if the SMS was successfully sent, false otherwise.
 */
bool sendSmsSerialGSM(const char *phoneNumber, const char *message);

/**
 * @brief Receives an SMS message from the GSM module and parses it into an SmsInfo structure.
 *
 * This function sends the AT command to list SMS messages in storage. It checks for 
 * any unread messages and parses the first one found into the provided SmsInfo structure. 
 * If an SMS is successfully received, it is deleted from the GSM module's storage.
 *
 * @param sms Pointer to an SmsInfo structure where the parsed SMS data will be stored. 
 *            This structure will be populated with the index, origin, datetime, and message content 
 *            of the received SMS.
 *
 * @return true if a new SMS was successfully received, parsed, and deleted from the GSM module; 
 *         false if no SMS was found or if there was an error in the process.
 */
bool receiveSmsSerialGSM(SmsInfo* sms);

/**
 * @brief Places a call to a specified phone number using the GSM module, monitors the call process, 
 * and hangs up after a specified duration if the call is not answered.
 * 
 * Sends an AT command to initiate a call to a provided phone number, monitors the call status,
 * and handles various possible responses such as call errors, busy signal, hang-up, etc.
 * 
 * @param phoneNumber The phone number to call.
 * @param hangUpDelay Time in milliseconds to wait before hanging up after the call connects (default is 1000 ms).
 * @param noAnswerTimeout Time in milliseconds to wait before hanging up if the call is not answered (default is 20000 ms).
 * @return true if the call was successfully initiated and handled, false otherwise.
 */
bool startCallSerialGSM(const char *phoneNumber, unsigned long hangUpDelay = 2000, unsigned long noAnswerTimeout = 20000);

/**
 * @brief Checks for incoming calls and rejects them, saving call details to a struct.
 * 
 * This function sends the AT+CLCC command to check for active or incoming calls.
 * If an incoming call is detected, it saves the call details (such as caller ID and status) 
 * into the provided `CallInfo` structure. After detection, it automatically rejects the 
 * call by sending the `ATH` command, which hangs up the call.
 * 
 * @param call Pointer to a `CallInfo` structure where the call details will be stored.
 *             The `call->id` will be set to a value greater than or equal to 0 if a call is detected.
 * @return true if an incoming call was detected and rejected, false if no incoming call was detected or
 *         if the check failed.
 */
bool receiveCallSerialGSM(CallInfo* call);

#endif
