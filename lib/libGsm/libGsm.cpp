#include "libGsm.h"

extern g_vars_t * g_vars_ptr;
extern g_config_t * g_config_ptr;

HardwareSerial SerialGSM(1);

void updateSerialGSM() {
    while (Serial.available()) {
        SerialGSM.write(Serial.read());
    }

    while (SerialGSM.available()) {
        Serial.write(SerialGSM.read());
    }
}

bool waitForCorrectResponseGSM(const char * command, const char * expectedResponse, String * receivedResponse, unsigned long timeout) {
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
    esplogW(TAG_LIB_GSM, "(waitForCorrectResponseGSM)", "GSM didnt responded in time!\n");
    return false;
}

bool initSerialGSM() {
    SerialGSM.begin(GSM_BAUDRATE, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
    SerialGSM.setTimeout(GSM_TIMEOUT);

    #ifdef GSM_RST_PIN
    pinMode(GSM_RST_PIN, OUTPUT);
    digitalWrite(GSM_RST_PIN, HIGH);
    #endif

    #ifdef GSM_PWR_PIN
    // i guess its empty in this lib?
    #endif

    #ifdef GSM_RI_PIN
    // i guess its empty in this lib?
    #endif

    #ifdef GSM_DTR_PIN
    // i guess its empty in this lib?
    #endif

    String response;
    String simCode;
    String registrationStatus;
    String signalQuality;
    int ret = true;

    esplogI(TAG_LIB_GSM, "(initSerialGSM)", "GSM INITIALISATION!");

    // initialisation
    if (!ret || !waitForCorrectResponseGSM("AT", "OK", &response, 10000)) {
        esplogW(TAG_LIB_GSM, "(initSerialGSM)", "Failed finding GSM module!");
        ret = false;
    } else {
        esplogI(TAG_LIB_GSM, "(initSerialGSM)", "GSM module found!");
    } if (!ret || !waitForCorrectResponseGSM("AT+CCID", "OK", &response)) {
        esplogW(TAG_LIB_GSM, "(initSerialGSM)", "Failed finding SIM!");
        ret = false;
    } else {
        int startIdx = response.indexOf("AT+CCID");
        if (startIdx >= 0) {
            int simCodeStart = response.indexOf("\r\n", startIdx) + 2;
            int simCodeEnd = response.indexOf("\r\n", simCodeStart);
            simCode = response.substring(simCodeStart, simCodeEnd);
        }
        esplogI(TAG_LIB_GSM, "(initSerialGSM)", "GSM SIM inserted!\n - SIM code: %s", simCode.c_str());
    } if (!ret || !waitForCorrectResponseGSM("AT+CREG?", "0,1", &response, 60000)) {
        esplogW(TAG_LIB_GSM, "(initSerialGSM)", "Failed to register to network!");
        ret = false;
    } else {
        int startIdx = response.indexOf("+CREG: ");
        if (startIdx >= 0) {registrationStatus = response.substring(startIdx + 7, response.indexOf("\r", startIdx));}
        esplogI(TAG_LIB_GSM, "(initSerialGSM)", "GSM SIM ready and registered!\n - registration status: %s", registrationStatus.c_str());
    } if (!ret || !waitForCorrectResponseGSM("AT+CSQ", "OK", &response)) {
        esplogW(TAG_LIB_GSM, "(initSerialGSM)", "Failed to get GSM signal info!");
        ret = false;
    } else {
        int startIdx = response.indexOf("+CSQ: ");
        if (startIdx >= 0) {
            int endIdx = response.indexOf(",", startIdx);
            signalQuality = response.substring(startIdx + 6, endIdx);
        }
        esplogI(TAG_LIB_GSM, "(initSerialGSM)", "GSM has signal!\n - signal quality: %s", signalQuality.c_str());
    }

    // setup
    if (!ret || !waitForCorrectResponseGSM("AT+CMGF=1", "OK", &response)) {
    } else {
        esplogI(TAG_LIB_GSM, "(initSerialGSM)", "GSM TEXT mode set successfully!");
    }if (!waitForCorrectResponseGSM("AT+CMGDA=\"DEL ALL\"", "OK", &response)) {
        return false;
    } else {
        esplogI(TAG_LIB_GSM, "(initSerialGSM)", "All SMS deleted successfully!");
    }
    // if (!ret || !waitForCorrectResponseGSM("AT+CLIP=1", "OK", &response)) {
    //     esplogW(TAG_LIB_GSM, "(initSerialGSM)", "Failed to set up caller ID notifications!");
    //     return false;
    // } else {
    //     esplogI(TAG_LIB_GSM, "(initSerialGSM)", "Caller ID notifications set up successfully!");
    // } 

    if (!ret) {
        return false;
    } else {
        return true;
    }
}

#ifdef GSM_SLEEP
bool sleepSerialGSM(bool sleep) {
    String response;

    if (sleep) {
        esplogI(TAG_LIB_GSM, "(sleepSerialGSM)", "Entering sleep mode!");
        if (waitForCorrectResponseGSM("AT+CSCLK=1", "OK", &response)) {
            esplogI(TAG_LIB_GSM, "(sleepSerialGSM)", "GSM module is now in sleep mode.");
            return true;
        } else {
            esplogW(TAG_LIB_GSM, "(sleepSerialGSM)", "Failed to put GSM module into sleep mode.");
            return false;
        }
    } else {
        esplogI(TAG_LIB_GSM, "(sleepSerialGSM)", "Leaving sleep mode!");
        SerialGSM.println("AT");
        delay(5000);
        if (waitForCorrectResponseGSM("AT+CSCLK=0", "OK", &response)) {
            esplogI(TAG_LIB_GSM, "(sleepSerialGSM)", "GSM module woke up from sleep.");
            return true;
        } else {
            esplogW(TAG_LIB_GSM, "(sleepSerialGSM)", "Failed to wake up GSM module.");
            return false;
        }
    }
}
#endif

#ifdef GSM_PWRDN
bool powerControlSerialGSM(bool power) {
    String response;

    if (!power) {
        esplogI(TAG_LIB_GSM, "(powerControlSerialGSM)", "Powering off GSM module!");
        if (waitForCorrectResponseGSM("AT+CPOWD=1", "NORMAL POWER DOWN", &response)) {
            esplogI(TAG_LIB_GSM, "(powerControlSerialGSM)", "GSM module powered off successfully.");
            return true;
        } else {
            esplogW(TAG_LIB_GSM, "(powerControlSerialGSM)", "Failed to power off the GSM module.");
            return false;
        }
    } else {
        esplogI(TAG_LIB_GSM, "(powerControlSerialGSM)", "Powering on GSM module!");
        SerialGSM.println("AT");
        delay(1000);

        if (waitForCorrectResponseGSM("AT", "OK", &response)) {
            esplogI(TAG_LIB_GSM, "(powerControlSerialGSM)", "GSM module powered on successfully.");
            return true;
        } else {
            esplogW("TAG_LIB_GSM, "(powerControlSerialGSM)", Failed to power on the GSM module. Check hardware reset.");
            return false;
        }
    }
}
#endif

#ifdef GSM_RST_PIN
bool resetSerialGSM() {
    esplogI(TAG_LIB_GSM, "(resetSerialGSM)", "Reseting GSM module!");

    SerialGSM.end();
    digitalWrite(GSM_RST_PIN, LOW);
    delay(50);
    digitalWrite(GSM_RST_PIN, HIGH);
    return initSerialGSM();
}
#endif

bool getRssiGSM(int * rssi, String * rssi_str) {
    String response;
    String signalQuality;

    if (!waitForCorrectResponseGSM("AT+CSQ", "OK", &response)) {
        esplogW(TAG_LIB_GSM, "(getRssiGSM)", "Failed to get GSM signal info!");
        return false;
    } else {
        int startIdx = response.indexOf("+CSQ: ");
        if (startIdx >= 0) {
            int endIdx = response.indexOf(",", startIdx);
            signalQuality = response.substring(startIdx + 6, endIdx);
            
            if (rssi != NULL) {
                *rssi = response.substring(startIdx + 6, endIdx).toInt();
            }

            if (rssi_str != NULL) {
                *rssi_str = String(signalQuality.c_str());
            }
        }
        esplogI(TAG_LIB_GSM, "(getRssiGSM)", "GSM RSSI: %d (str: %s)", signalQuality.toInt(), signalQuality.c_str());
        return true;
    }
}

bool sendSmsSerialGSM(const char *phoneNumber, const char *message) {
    String response;

    esplogI(TAG_LIB_GSM, "(sendSmsSerialGSM)", "Sending a SMS message!\n - %s\n - %s", phoneNumber, message);

    String smsCommand = "AT+CMGS=\"";
    smsCommand += phoneNumber;
    smsCommand += "\"";
    
    if (!waitForCorrectResponseGSM(smsCommand.c_str(), ">", &response)) {
        esplogW(TAG_LIB_GSM, "(sendSmsSerialGSM)", "Failed to send phone number for SMS!");
        return false;
    }

    SerialGSM.print(message);
    SerialGSM.write(26);

    if (waitForCorrectResponseGSM("", "OK", &response, 10000)) {
        esplogI(TAG_LIB_GSM, "(sendSmsSerialGSM)", "SMS successfully sent!");
        return true;
    } else {
        esplogW(TAG_LIB_GSM, "(sendSmsSerialGSM)", "Failed to send SMS!");
        return false;
    }
}

bool receiveSmsSerialGSM(SmsInfo* sms) {
    String response;

    if (!waitForCorrectResponseGSM("AT+CMGL", "OK", &response)) {
        esplogW(TAG_LIB_GSM, "(receiveSmsSerialGSM)", "Failed to check for received SMS!");
        return false;
    }

    *sms = parseCMGLResponse(response);

    if (sms->index < 0) {
        return false;
    } else {
        esplogI(TAG_LIB_GSM, "(receiveSmsSerialGSM)", "New SMS received!");

        String deleteCommand = "AT+CMGD=" + String(sms->index);
        String deleteResponse;
        if (waitForCorrectResponseGSM(deleteCommand.c_str(), "OK", &deleteResponse)) {
            esplogI(TAG_LIB_GSM, "(receiveSmsSerialGSM)", "New SMS was successfully saved to struct and deleted from GSM!");
        } else {
            esplogW(TAG_LIB_GSM, "(receiveSmsSerialGSM)", "Failed to delete SMS!");
        }
        return true;
    }
}

bool startCallSerialGSM(const char *phoneNumber, unsigned long hangUpDelay, unsigned long noAnswerTimeout) {
    String response;
    
    esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Calling number!\n - %s", phoneNumber);

    // Initiate the call
    String callCommand = "ATD";
    callCommand += phoneNumber;
    callCommand += ";";

    if (!waitForCorrectResponseGSM(callCommand.c_str(), "OK", &response)) {
        esplogW(TAG_LIB_GSM, "(startCallSerialGSM)", "Failed to initiate the call!");
        return false;
    } else {
        esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Call initiated successfully!");
    }

    unsigned long startTime = millis();
    bool callConnected = false;
    bool callEnded = false;

    while (!callEnded) {
        // Periodically send AT+CLCC to check the call status
        if (!waitForCorrectResponseGSM("AT+CLCC", "OK", &response)) {
            esplogW(TAG_LIB_GSM, "(startCallSerialGSM)", "No response received for AT+CLCC command!");
            waitForCorrectResponseGSM("ATH", "OK", &response);
            return false;
        }

        if (response.indexOf("+CLCC") >= 0) {
            CallInfo callInfo = parseCLCCResponse(response);

            switch (callInfo.stat) {
                case 0:  // Call is active
                    esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Calling...");
                    callConnected = true;
                    break;
                case 1:  // Held
                    esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Held...");
                    break;
                case 2:  // Dialing
                    esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Dialing...");
                    break;
                case 3:  // Alerting (ringing)
                    esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Ringing...");
                    break;
                case 4:  // Incoming call (not applicable here)
                    esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Incoming call (should not happen).");
                    break;
                case 6:  // Call disconnected
                    esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Call disconnected by the remote party.");
                    callEnded = true;
                    break;
                default:
                    esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Unknown call status.");
                    break;
            }
        }

        // Check if the call has timed out without being answered
        if (!callConnected && (millis() - startTime >= noAnswerTimeout)) {
            esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "No answer, hanging up the call!");
            waitForCorrectResponseGSM("ATH", "OK", &response);
            callEnded = true;
        }

        // If the call is connected, hang up after the specified delay
        if (callConnected && (millis() - startTime >= hangUpDelay)) {
            esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Hanging up the call after the delay!");
            waitForCorrectResponseGSM("ATH", "OK", &response);
            callEnded = true;
        }
    }

    if (callConnected) {
        esplogI(TAG_LIB_GSM, "(startCallSerialGSM)", "Call ended successfully!");
        return true;
    } else {
        esplogW(TAG_LIB_GSM, "(startCallSerialGSM)", "Call was not connected or failed!");
        return false;
    }
}

bool receiveCallSerialGSM(CallInfo* call) {
    String response;

    if (!waitForCorrectResponseGSM("AT+CLCC", "OK", &response)) {
        esplogW(TAG_LIB_GSM, "(receiveCallSerialGSM)", "Failed to check for incoming calls!");
        return false;
    }

    *call = parseCLCCResponse(response);
    
    if (call->id < 0) {
        return false;
    } else {
        esplogI(TAG_LIB_GSM, "(receiveCallSerialGSM)", "Incoming call detected!");

        String hangupResponse;
        if (waitForCorrectResponseGSM("ATH", "OK", &hangupResponse)) {
            esplogI(TAG_LIB_GSM, "(receiveCallSerialGSM)", "Incoming call rejected successfully!");
        } else {
            esplogW(TAG_LIB_GSM, "(receiveCallSerialGSM)", "Failed to reject the incoming call!");
        }
        return true;
    }
}
