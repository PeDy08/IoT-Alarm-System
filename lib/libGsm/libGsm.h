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
#include <PCF8574.h>

#include "utils.h"
#include "mainAppDefinitions.h"

#define GSM_RX_PIN 27
#define GSM_TX_PIN 14
#define GSM_PWR_PIN 1 // <- will be at pcf8574
#define GSM_DTR_PIN 0 // <- will be at pcf8574
#define GSM_RI_PIN 2 // <- will be at pcf8574
#define GSM_BAUDRATE 9600
#define GSM_TIMEOUT 1000
// #define GSM_PIN ""  // <- SIMCARD PASSWORD not implemented (command: AT+CPIN=****)

// volatile functions
// #define GSM_SLEEP
// #define GSM_PWRDN

extern HardwareSerial SerialGSM;
extern PCF8574 gpio_extender;

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

/**
 * @brief Parses the response from the CLCC command to extract call information.
 *
 * This function processes a response from the `+CLCC` command (Call List Command) and extracts various call details, 
 * including call ID, direction, status, mode, multiparty information, caller's phone number, call type, and 
 * optional alpha identifier. If no call is detected in the response, default values are assigned to all fields.
 *
 * @param response A string containing the response from the `+CLCC` command. 
 *                 The response should be in the format `+CLCC: <id>,<dir>,<stat>,<mode>,<mpty>,"<number>",<type>,"<alphaID>"`.
 *                 This function expects the response to contain the `+CLCC:` prefix followed by the call information.
 * 
 * @return CallInfo A structure containing the parsed call information:
 *                  - `id`: The call ID (integer)
 *                  - `dir`: The direction of the call (integer)
 *                  - `stat`: The status of the call (integer)
 *                  - `mode`: The mode of the call (integer)
 *                  - `mpty`: The multiparty flag (integer)
 *                  - `number`: The phone number of the caller (string)
 *                  - `type`: The type of call (integer)
 *                  - `alphaID`: The alpha identifier for the call (string)
 *
 * @details This function checks the response for the presence of `+CLCC:` to determine if a call has been detected.
 *          If no call is found, it returns a `CallInfo` object with default values for all fields.
 *          The function extracts the relevant fields by parsing the response string and handling optional fields
 *          like `number`, `type`, and `alphaID` which may or may not be present in the response.
 *
 * @code
 * String response = "+CLCC: 1,1,4,0,1,\"+1234567890\",129,\"Test Caller\"";
 * CallInfo call = parseCLCCResponse(response);
 * Serial.println(call.id);     // 1
 * Serial.println(call.number); // "+1234567890"
 * Serial.println(call.alphaID); // "Test Caller"
 * @endcode
 */
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

/**
 * @brief Parses the response from the CMGL command to extract SMS information.
 *
 * This function processes a response from the `+CMGL` command (List Messages Command) and extracts various SMS details, 
 * including the SMS index, status, origin (sender's phone number), datetime, and message content. If no SMS is found 
 * in the response, it returns a `SmsInfo` object with default values.
 *
 * @param response A string containing the response from the `+CMGL` command.
 *                 The response should be in the format `+CMGL: <index>,<status>,"<origin>","<datetime>"<message>`, 
 *                 where `<index>` is the SMS index, `<status>` is the message status, `<origin>` is the sender's number, 
 *                 `<datetime>` is the timestamp, and `<message>` is the content of the SMS.
 * 
 * @return SmsInfo A structure containing the parsed SMS information:
 *                  - `index`: The SMS index (integer)
 *                  - `status`: The status of the SMS (string)
 *                  - `origin`: The origin phone number (string)
 *                  - `datetime`: The datetime when the SMS was received (string)
 *                  - `message`: The content of the SMS (string)
 *
 * @details This function checks the response for the presence of `+CMGL:` to determine if an SMS has been detected.
 *          If no SMS is found, it returns a `SmsInfo` object with default values. The function then extracts the relevant
 *          fields, including the SMS index, status, origin number, datetime, and the message content. It trims any extra
 *          newline characters from the message content.
 *
 * @code
 * String response = "+CMGL: 1,\"REC READ\",\"+1234567890\",\"12/12/21,12:00:00+00\",Hello World!";
 * SmsInfo sms = parseCMGLResponse(response);
 * Serial.println(sms.index);      // 1
 * Serial.println(sms.status);     // "REC READ"
 * Serial.println(sms.origin);     // "+1234567890"
 * Serial.println(sms.datetime);  // "12/12/21,12:00:00+00"
 * Serial.println(sms.message);   // "Hello World!"
 * @endcode
 */
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
 * @brief Waits for a specific response from the GSM module after sending a command.
 *
 * This function sends a command to the GSM module and waits for a response within a specified timeout period. 
 * If the response from the GSM module contains the expected string, the function returns `true`; otherwise, 
 * it returns `false` after the timeout period. The received response is stored in the provided `receivedResponse` 
 * variable.
 *
 * @param command The command string to send to the GSM module.
 * @param expectedResponse The string that the function expects to receive from the GSM module.
 * @param receivedResponse A pointer to a `String` where the received response will be stored.
 * @param timeout The maximum time (in milliseconds) to wait for the expected response. Default is 5000 ms.
 * 
 * @return `true` if the expected response is received within the timeout period, otherwise `false`.
 *
 * @details This function continuously sends the command to the GSM module and reads the response. If the response 
 *          contains the expected string, it returns `true`. If the response does not contain the expected string 
 *          within the timeout period, it logs a warning and returns `false`. This function is useful for ensuring 
 *          that the GSM module responds correctly to a command.
 *
 * @code
 * String response;
 * if (waitForCorrectResponseGSM("AT", "OK", &response)) {
 *     Serial.println("GSM module responded correctly!");
 * } else {
 *     Serial.println("GSM module did not respond in time.");
 * }
 * @endcode
 */
bool waitForCorrectResponseGSM(const char * command, const char * expectedResponse, String * receivedResponse, unsigned long timeout = 5000);

/**
 * @brief Updates the communication between the serial monitor and the GSM module.
 *
 * This function handles data transmission between the serial monitor (`Serial`) and the GSM module (`SerialGSM`). 
 * It reads data from the serial monitor and sends it to the GSM module. It also reads data from the GSM module and 
 * sends it back to the serial monitor. This ensures bidirectional communication between the two devices.
 *
 * @details The function continuously checks if there is data available in the serial monitor. If data is available, 
 *          it is read and forwarded to the GSM module. Similarly, it checks if there is data available from the GSM 
 *          module and sends it to the serial monitor. This function should be called periodically in the loop to keep 
 *          the communication active.
 *
 * @code
 * // In the main loop, call the function to keep the communication active
 * updateSerialGSM();
 * @endcode
 */
void updateSerialGSM();

/**
 * @brief Initializes the GSM module and checks its status.
 *
 * This function initializes the GSM serial communication, configures the necessary pins, 
 * and performs a series of AT commands to ensure the GSM module and SIM card are functional 
 * and connected to the network. The function also sets up text mode for SMS and deletes all 
 * existing SMS messages in the module's memory.
 *
 * @return `true` if the GSM module is successfully initialized and configured, otherwise `false`.
 *
 * @details 
 * - Initializes the serial communication with the GSM module.
 * - Configures the optional GSM reset (RST), power (PWR), ring indicator (RI), and data terminal 
 *   ready (DTR) pins if defined, though they may not be utilized in the current library.
 * - Sends the following AT commands to the GSM module:
 *   - "AT" to check module availability.
 *   - "AT+CCID" to verify SIM card presence and extract its code.
 *   - "AT+CREG?" to check network registration status.
 *   - "AT+CSQ" to check signal quality.
 *   - "AT+CMGF=1" to set the SMS mode to text.
 *   - "AT+CMGDA=\"DEL ALL\"" to delete all stored SMS messages.
 * - Logs the results of these operations, including errors if any step fails.
 *
 * This function is essential for ensuring that the GSM module is ready for further operations such as 
 * sending or receiving SMS and making calls. If any of the checks or initializations fail, the function 
 * will return `false`.
 *
 * @code
 * if (initSerialGSM()) {
 *     Serial.println("GSM module initialized successfully.");
 * } else {
 *     Serial.println("Failed to initialize GSM module.");
 * }
 * @endcode
 */
bool initSerialGSM();

#ifdef GSM_SLEEP
/**
 * @brief Puts the GSM module into sleep mode or wakes it up.
 *
 * This function allows the GSM module to enter sleep mode to conserve power or 
 * wake up from sleep mode for normal operation. When the module is in sleep mode, 
 * it will not respond to incoming commands or messages until it is woken up.
 *
 * @param sleep A boolean value to control the sleep state:
 *              - `true` to put the GSM module into sleep mode.
 *              - `false` to wake the GSM module from sleep mode.
 * 
 * @return `true` if the GSM module successfully enters or exits sleep mode, otherwise `false`.
 *
 * @details 
 * - If `sleep` is `true`, the function sends the "AT+CSCLK=1" command to the GSM module, which puts it into sleep mode.
 * - If `sleep` is `false`, the function sends the "AT+CSCLK=0" command to wake the GSM module from sleep mode.
 * - The function waits for an "OK" response from the GSM module to confirm the operation.
 * - If the operation is successful, it logs the corresponding message.
 *
 * @code
 * if (sleepSerialGSM(true)) {
 *     Serial.println("GSM module is now in sleep mode.");
 * } else {
 *     Serial.println("Failed to put GSM module into sleep mode.");
 * }
 * @endcode
 */
bool sleepSerialGSM(bool sleep);
#endif

#ifdef GSM_PWRDN
/**
 * @brief Powers the GSM module on or off.
 *
 * This function allows for controlling the power state of the GSM module. It sends commands
 * to either power off or power on the module, depending on the provided `power` parameter.
 * The function waits for a confirmation response from the GSM module before returning.
 *
 * @param power A boolean value to control the power state of the GSM module:
 *              - `false` to power off the GSM module.
 *              - `true` to power on the GSM module.
 * 
 * @return `true` if the GSM module was successfully powered on or off, otherwise `false`.
 *
 * @details 
 * - If `power` is `false`, the function sends the "AT+CPOWD=1" command to power off the GSM module, and waits for the "NORMAL POWER DOWN" response.
 * - If `power` is `true`, the function sends the "AT" command to power on the GSM module and waits for the "OK" response.
 * - If the operation is successful, the function logs an informational message. If there’s a failure, it logs a warning message.
 *
 * @code
 * if (powerControlSerialGSM(true)) {
 *     Serial.println("GSM module powered on successfully.");
 * } else {
 *     Serial.println("Failed to power on the GSM module.");
 * }
 * @endcode
 */
bool powerControlSerialGSM(bool power);
#endif

#ifdef GSM_RST_PIN
/**
 * @brief Resets the GSM module and reinitializes the serial connection.
 *
 * This function performs a reset of the GSM module by toggling the reset pin and reinitializes
 * the serial communication with the GSM module. It first powers off the GSM module using the
 * reset pin, waits briefly, then powers it back on, followed by reinitializing the serial connection.
 *
 * @return `true` if the GSM module is successfully reset and reinitialized, otherwise `false`.
 *
 * @details
 * - The function begins by logging the reset action.
 * - The `SerialGSM.end()` command is called to stop the current serial communication.
 * - The reset pin (`GSM_RST_PIN`) is toggled: it is set to LOW for 50 milliseconds and then set to HIGH.
 * - After resetting, the function calls `initSerialGSM()` to reinitialize the GSM module and its serial communication.
 * 
 * @code
 * if (resetSerialGSM()) {
 *     Serial.println("GSM module reset successfully.");
 * } else {
 *     Serial.println("Failed to reset the GSM module.");
 * }
 * @endcode
 */
bool resetSerialGSM();
#endif

/**
 * @brief Retrieves the GSM signal strength (RSSI) from the GSM module.
 *
 * This function sends the `AT+CSQ` command to the GSM module to request the signal strength information. 
 * It parses the response to extract the Received Signal Strength Indicator (RSSI) value and optionally 
 * stores it in both an integer and a string variable. The function returns `true` if the signal strength 
 * is successfully retrieved, otherwise `false`.
 *
 * @param rssi A pointer to an integer where the RSSI value (signal strength) will be stored.
 *             If this parameter is `NULL`, the RSSI value will not be saved.
 * @param rssi_str A pointer to a `String` where the RSSI value as a string will be stored.
 *                 If this parameter is `NULL`, the string representation of the RSSI value will not be saved.
 *
 * @return `true` if the signal strength is successfully retrieved, otherwise `false`.
 *
 * @details
 * - The function sends the `AT+CSQ` command to the GSM module to retrieve signal strength information.
 * - The response is parsed to extract the RSSI value, which is returned as an integer and optionally
 *   as a string.
 * - The function logs the RSSI value as both an integer and a string for debugging purposes.
 *
 * @code
 * int rssi;
 * String rssi_str;
 * if (getRssiGSM(&rssi, &rssi_str)) {
 *     Serial.printf("GSM Signal Strength: %d (RSSI as string: %s)\n", rssi, rssi_str.c_str());
 * } else {
 *     Serial.println("Failed to retrieve GSM signal strength.");
 * }
 * @endcode
 */
bool getRssiGSM(int * rssi = NULL, String * rssi_str = NULL);

/**
 * @brief Sends an SMS message via the GSM module.
 *
 * This function sends an SMS to a specified phone number by sending the `AT+CMGS` command to the GSM module, 
 * followed by the message content. It waits for the `>` prompt before sending the message and for the 
 * `OK` response to confirm successful sending. The function logs the progress and success/failure of the operation.
 *
 * @param phoneNumber The phone number to which the SMS will be sent. It should be in the international format (e.g., "+123456789").
 * @param message The content of the SMS message to be sent.
 *
 * @return `true` if the SMS was successfully sent, otherwise `false`.
 *
 * @details
 * - The function constructs an `AT+CMGS` command with the phone number and waits for the `>` prompt.
 * - After the prompt is received, it sends the message and terminates it with the Ctrl+Z character (`0x1A` or `26`).
 * - It then waits for an `OK` response within a timeout of 10 seconds to confirm the successful sending of the SMS.
 * - The function logs messages indicating the start, success, or failure of the SMS sending operation.
 *
 * @code
 * const char *phoneNumber = "+123456789";
 * const char *message = "Hello, this is a test SMS!";
 * if (sendSmsSerialGSM(phoneNumber, message)) {
 *     Serial.println("SMS sent successfully.");
 * } else {
 *     Serial.println("Failed to send SMS.");
 * }
 * @endcode
 */
bool sendSmsSerialGSM(const char *phoneNumber, const char *message);

/**
 * @brief Receives an SMS message from the GSM module.
 *
 * This function queries the GSM module for any received SMS messages using the `AT+CMGL` command,
 * and processes the response to retrieve the message details. If a new SMS is received, it is stored in 
 * the provided `SmsInfo` structure. The function also deletes the message from the GSM module after it is processed.
 *
 * @param sms A pointer to an `SmsInfo` structure where the received SMS details (like sender number, 
 *            message content, etc.) will be saved.
 *
 * @return `true` if a new SMS was successfully received, parsed, and deleted from the GSM module, 
 *         otherwise `false` if no new SMS was found or if there was an error during the process.
 *
 * @details
 * - The function sends the `AT+CMGL` command to the GSM module to check for any received SMS messages.
 * - If a message is found, it is parsed and stored in the provided `SmsInfo` structure.
 * - The function then sends the `AT+CMGD` command to delete the SMS from the GSM module after processing it.
 * - The function logs the status of each step, including whether the SMS was successfully received and deleted.
 *
 * @code
 * SmsInfo sms;
 * if (receiveSmsSerialGSM(&sms)) {
 *     Serial.println("New SMS received:");
 *     Serial.println("Sender: " + sms.origin);
 *     Serial.println("Message: " + sms.message);
 * } else {
 *     Serial.println("No new SMS or failed to receive.");
 * }
 * @endcode
 */
bool receiveSmsSerialGSM(SmsInfo* sms);

/**
 * @brief Initiates a call to a specified phone number and manages the call status.
 *
 * This function initiates a call to the specified phone number using the GSM module. It monitors the call status 
 * by periodically sending the `AT+CLCC` command and logs the call progress, such as dialing, ringing, or call connected. 
 * If the call is answered, it will be automatically ended after the specified delay (`hangUpDelay`). If no answer is received 
 * within the specified timeout (`noAnswerTimeout`), the function will hang up the call. It handles call status updates and 
 * manages hanging up the call when appropriate.
 *
 * @param phoneNumber The phone number to call (in E.164 format, e.g., "+1234567890").
 * @param hangUpDelay The time (in milliseconds) to wait after the call is connected before hanging up.
 * @param noAnswerTimeout The time (in milliseconds) to wait for the call to be answered before hanging up due to no answer.
 *
 * @return `true` if the call was successfully initiated, connected, and ended; `false` if the call could not be initiated,
 *         was not answered, or failed for any other reason.
 *
 * @details
 * - The function starts by sending the `ATD<phoneNumber>;` command to initiate the call.
 * - It then enters a loop where it repeatedly sends the `AT+CLCC` command to check the status of the call.
 * - Depending on the call status, the function logs the current call state (e.g., dialing, ringing, connected, disconnected).
 * - If the call is not answered within the `noAnswerTimeout`, the function will hang up the call.
 * - If the call is answered, the function will hang up the call after waiting for the `hangUpDelay`.
 * - The function ends when either the call is disconnected or the specified timeouts are reached.
 *
 * @code
 * const char *phoneNumber = "+1234567890";
 * unsigned long hangUpDelay = 5000; // 5 seconds
 * unsigned long noAnswerTimeout = 10000; // 10 seconds
 * if (startCallSerialGSM(phoneNumber, hangUpDelay, noAnswerTimeout)) {
 *     Serial.println("Call ended successfully!");
 * } else {
 *     Serial.println("Call failed!");
 * }
 * @endcode
 */
bool startCallSerialGSM(const char *phoneNumber, unsigned long hangUpDelay = 2000, unsigned long noAnswerTimeout = 20000);

/**
 * @brief Checks for an incoming call and rejects it if detected.
 *
 * This function checks for an incoming call by sending the `AT+CLCC` command to the GSM module. If an incoming call 
 * is detected, it parses the call information and logs the event. It then attempts to reject the incoming call by 
 * sending the `ATH` command. If the call is successfully rejected, the function returns `true`; otherwise, it returns `false`.
 *
 * @param call A pointer to a `CallInfo` structure that will be populated with the incoming call details, such as the 
 *             call ID and status. If no incoming call is detected, the structure will not be updated.
 *
 * @return `true` if an incoming call was detected and rejected successfully; `false` if no incoming call was detected 
 *         or if the call rejection failed.
 *
 * @details
 * - The function starts by sending the `AT+CLCC` command to check for any incoming calls.
 * - If an incoming call is detected, it parses the response to extract the call information.
 * - If the call ID is valid (greater than or equal to 0), the function proceeds to reject the call by sending the `ATH` command.
 * - The function logs the result of the rejection attempt.
 *
 * @code
 * CallInfo callInfo;
 * if (receiveCallSerialGSM(&callInfo)) {
 *     Serial.println("Incoming call detected and rejected successfully.");
 * } else {
 *     Serial.println("No incoming call or failed to reject the call.");
 * }
 * @endcode
 */
bool receiveCallSerialGSM(CallInfo* call);

#endif
