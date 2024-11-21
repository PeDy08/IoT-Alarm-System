#include "libGsm.h"

HardwareSerial SerialGSM(1);

void updateSerialGSM() {
    while (Serial.available()) {
        SerialGSM.write(Serial.read());
    }

    while (SerialGSM.available()) {
        Serial.write(SerialGSM.read());
    }
}

/**
 * @brief Sends an AT command to the GSM module and waits for the expected response.
 * 
 * This function repeatedly sends an AT command to the GSM module and checks if the
 * expected response is received. If the response contains the expected string, it
 * returns true; otherwise, it will keep trying until a timeout occurs.
 * 
 * @param command The AT command to send to the GSM module.
 * @param expectedResponse The expected response to the AT command.
 * @param receivedResponse Pointer to a String that will store the received response.
 * @param timeout The time (in milliseconds) to wait for the response (default is 6000 ms).
 * @return true if the expected response was received, false if the timeout occurred.
 */
bool waitForCorrectResponseGSM(const char * command, const char * expectedResponse, String * receivedResponse, unsigned long timeout = 5000) {
    while (SerialGSM.available()) {
        SerialGSM.read();
    }

    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
        SerialGSM.println(command);
        *receivedResponse = SerialGSM.readString();
        if (receivedResponse->indexOf(expectedResponse) >= 0) {
            // esplogI(" (libGsm): SENT: %s\n", command);
            // esplogI(" (libGsm): RECEIVED: %s\n", *receivedResponse);
            return true;
        }
    }
    esplogW(" (libGsm): GSM didnt responded in time!\n");
    return false;
}

bool initSerialGSM() {
    SerialGSM.begin(GSM_BAUDRATE, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
    SerialGSM.setTimeout(GSM_TIMEOUT);

    #ifdef GSM_RST_PIN
    pinMode(GSM_RST_PIN, OUTPUT);
    digitalWrite(GSM_RST_PIN, HIGH);
    #endif

    Serial.println("-------------------------------------------\nGSM INITIALISATION:\n");

    String response;
    String simCode;
    String registrationStatus;
    String signalQuality;
    int ret = true;

    // initialisation
    if (!ret || !waitForCorrectResponseGSM("AT", "OK", &response, 10000)) {
        esplogW(" (libGsm): Failed finding GSM module!\n");
        ret = false;
    } else {
        esplogI(" (libGsm): GSM module found!\n");
    } if (!ret || !waitForCorrectResponseGSM("AT+CCID", "OK", &response)) {
        esplogW(" (libGsm): Failed finding SIM!\n");
        ret = false;
    } else {
        int startIdx = response.indexOf("AT+CCID");
        if (startIdx >= 0) {
            int simCodeStart = response.indexOf("\r\n", startIdx) + 2;
            int simCodeEnd = response.indexOf("\r\n", simCodeStart);
            simCode = response.substring(simCodeStart, simCodeEnd);
        }
        esplogI(" (libGsm): GSM SIM inserted!\n - SIM code: %s\n", simCode.c_str());
    } if (!ret || !waitForCorrectResponseGSM("AT+CREG?", "0,1", &response, 60000)) {
        esplogW(" (libGsm): Failed to register to network!\n");
        ret = false;
    } else {
        int startIdx = response.indexOf("+CREG: ");
        if (startIdx >= 0) {registrationStatus = response.substring(startIdx + 7, response.indexOf("\r", startIdx));}
        esplogI(" (libGsm): GSM SIM ready and registered!\n - registration status: %s\n", registrationStatus.c_str());
    } if (!ret || !waitForCorrectResponseGSM("AT+CSQ", "OK", &response)) {
        esplogW(" (libGsm): Failed to get GSM signal info!\n");
        ret = false;
    } else {
        int startIdx = response.indexOf("+CSQ: ");
        if (startIdx >= 0) {
            int endIdx = response.indexOf(",", startIdx);
            signalQuality = response.substring(startIdx + 6, endIdx);
        }
        esplogI(" (libGsm): GSM has signal!\n - signal quality: %s\n", signalQuality.c_str());
    }

    // setup
    if (!ret || !waitForCorrectResponseGSM("AT+CMGF=1", "OK", &response)) {
    } else {
        esplogI(" (libGsm): GSM TEXT mode set successfully!\n");
    }if (!waitForCorrectResponseGSM("AT+CMGDA=\"DEL ALL\"", "OK", &response)) {
        return false;
    } else {
        esplogI(" (libGsm): All SMS deleted successfully!\n");
    }
    // if (!ret || !waitForCorrectResponseGSM("AT+CLIP=1", "OK", &response)) {
    //     esplogW(" (libGsm): Failed to set up caller ID notifications!\n");
    //     return false;
    // } else {
    //     esplogI(" (libGsm): Caller ID notifications set up successfully!\n");
    // } 

    if (!ret) {
        Serial.println("\nINITIALISATION FAILED\n-------------------------------------------\n");
        return false;
    } else {
        Serial.println("\nSUCCESSFULLY INITIALISED\n-------------------------------------------\n");
        return true;
    }
}

#ifdef GSM_SLEEP
bool sleepSerialGSM(bool sleep) {
    String response;

    if (sleep) {
        esplogI(" (libGsm): Entering sleep mode!\n");
        if (waitForCorrectResponseGSM("AT+CSCLK=1", "OK", &response)) {
            esplogI(" (libGsm): GSM module is now in sleep mode.\n");
            return true;
        } else {
            esplogW(" (libGsm): Failed to put GSM module into sleep mode.\n");
            return false;
        }
    } else {
        esplogI(" (libGsm): Leaving sleep mode!\n");
        SerialGSM.println("AT");
        delay(5000);
        if (waitForCorrectResponseGSM("AT+CSCLK=0", "OK", &response)) {
            esplogI(" (libGsm): GSM module woke up from sleep.\n");
            return true;
        } else {
            esplogW(" (libGsm): Failed to wake up GSM module.\n");
            return false;
        }
    }
}
#endif

#ifdef GSM_PWRDN
bool powerControlSerialGSM(bool power) {
    String response;

    if (!power) {
        esplogI(" (libGsm): Powering off GSM module!\n");
        if (waitForCorrectResponseGSM("AT+CPOWD=1", "NORMAL POWER DOWN", &response)) {
            esplogI(" (libGsm): GSM module powered off successfully.\n");
            return true;
        } else {
            esplogW(" (libGsm): Failed to power off the GSM module.\n");
            return false;
        }
    } else {
        esplogI(" (libGsm): Powering on GSM module!\n");
        SerialGSM.println("AT");
        delay(1000);

        if (waitForCorrectResponseGSM("AT", "OK", &response)) {
            esplogI(" (libGsm): GSM module powered on successfully.\n");
            return true;
        } else {
            esplogW(" (libGsm): Failed to power on the GSM module. Check hardware reset.\n");
            return false;
        }
    }
}
#endif

#ifdef GSM_RST_PIN
bool resetSerialGSM() {
    esplogI(" (libGsm): Reseting GSM module!\n");

    SerialGSM.end();
    digitalWrite(GSM_RST_PIN, LOW);
    delay(50);
    digitalWrite(GSM_RST_PIN, HIGH);
    return initSerialGSM();
}
#endif

bool sendSmsSerialGSM(const char *phoneNumber, const char *message) {
    String response;

    esplogI(" (libGsm): Sending a SMS message!\n - %s\n - %s\n", phoneNumber, message);

    String smsCommand = "AT+CMGS=\"";
    smsCommand += phoneNumber;
    smsCommand += "\"";
    
    if (!waitForCorrectResponseGSM(smsCommand.c_str(), ">", &response)) {
        esplogW(" (libGsm): Failed to send phone number for SMS!\n");
        return false;
    }

    SerialGSM.print(message);
    SerialGSM.write(26);

    if (waitForCorrectResponseGSM("", "OK", &response, 10000)) {
        esplogI(" (libGsm): SMS successfully sent!\n");
        return true;
    } else {
        esplogW(" (libGsm): Failed to send SMS!\n");
        return false;
    }
}

bool receiveSmsSerialGSM(SmsInfo* sms) {
    String response;

    if (!waitForCorrectResponseGSM("AT+CMGL", "OK", &response)) {
        esplogW(" (libGsm): Failed to check for received SMS!\n");
        return false;
    }

    *sms = parseCMGLResponse(response);

    if (sms->index < 0) {
        return false;
    } else {
        esplogI(" (libGsm): New SMS received!\n");

        String deleteCommand = "AT+CMGD=" + String(sms->index);
        String deleteResponse;
        if (waitForCorrectResponseGSM(deleteCommand.c_str(), "OK", &deleteResponse)) {
            esplogI(" (libGsm): New SMS was successfully saved to struct and deleted from GSM!\n");
        } else {
            esplogW(" (libGsm): Failed to delete SMS!\n");
        }
        return true;
    }
}


bool startCallSerialGSM(const char *phoneNumber, unsigned long hangUpDelay, unsigned long noAnswerTimeout) {
    String response;
    
    esplogI(" (libGsm): Calling number!\n - %s\n", phoneNumber);

    // Initiate the call
    String callCommand = "ATD";
    callCommand += phoneNumber;
    callCommand += ";";

    if (!waitForCorrectResponseGSM(callCommand.c_str(), "OK", &response)) {
        esplogW(" (libGsm): Failed to initiate the call!\n");
        return false;
    } else {
        esplogI(" (libGsm): Call initiated successfully!\n");
    }

    unsigned long startTime = millis();
    bool callConnected = false;
    bool callEnded = false;

    while (!callEnded) {
        // Periodically send AT+CLCC to check the call status
        if (!waitForCorrectResponseGSM("AT+CLCC", "OK", &response)) {
            esplogW(" (libGsm): No response received for AT+CLCC command!\n");
            waitForCorrectResponseGSM("ATH", "OK", &response);
            return false;
        }

        if (response.indexOf("+CLCC") >= 0) {
            CallInfo callInfo = parseCLCCResponse(response);

            switch (callInfo.stat) {
                case 0:  // Call is active
                    esplogI("Calling...");
                    callConnected = true;
                    break;
                case 1:  // Held
                    esplogI("Held...");
                    break;
                case 2:  // Dialing
                    esplogI("Dialing...");
                    break;
                case 3:  // Alerting (ringing)
                    esplogI("Ringing...");
                    break;
                case 4:  // Incoming call (not applicable here)
                    esplogI("Incoming call (should not happen).");
                    break;
                case 6:  // Call disconnected
                    esplogI("Call disconnected by the remote party.");
                    callEnded = true;
                    break;
                default:
                    esplogI("Unknown call status.");
                    break;
            }
        }

        // Check if the call has timed out without being answered
        if (!callConnected && (millis() - startTime >= noAnswerTimeout)) {
            esplogI(" (libGsm): No answer, hanging up the call!\n");
            waitForCorrectResponseGSM("ATH", "OK", &response);
            callEnded = true;
        }

        // If the call is connected, hang up after the specified delay
        if (callConnected && (millis() - startTime >= hangUpDelay)) {
            esplogI(" (libGsm): Hanging up the call after the delay!\n");
            waitForCorrectResponseGSM("ATH", "OK", &response);
            callEnded = true;
        }
    }

    if (callConnected) {
        esplogI(" (libGsm): Call ended successfully!\n");
        return true;
    } else {
        esplogW(" (libGsm): Call was not connected or failed!\n");
        return false;
    }
}

bool receiveCallSerialGSM(CallInfo* call) {
    String response;

    if (!waitForCorrectResponseGSM("AT+CLCC", "OK", &response)) {
        esplogW(" (libGsm): Failed to check for incoming calls!\n");
        return false;
    }

    *call = parseCLCCResponse(response);
    
    if (call->id < 0) {
        return false;
    } else {
        esplogI(" (libGsm): Incoming call detected!\n");

        String hangupResponse;
        if (waitForCorrectResponseGSM("ATH", "OK", &hangupResponse)) {
            esplogI(" (libGsm): Incoming call rejected successfully!\n");
        } else {
            esplogW(" (libGsm): Failed to reject the incoming call!\n");
        }
        return true;
    }
}
