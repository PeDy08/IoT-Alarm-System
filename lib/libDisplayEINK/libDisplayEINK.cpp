#include "libDisplayEINK.h"

#include <Arduino.h>
#include <SPI.h>

#define SPI_MOSI 16
#define SPI_MISO 4
#define SPI_CLK 17

#define EPD_CS 5
#define EPD_RST 19
#define EPD_DC 18
#define EPD_BUSY 23

#define Y_OFFSET 6
#define Y_OFFSET_8th_high 8
#define Y_OFFSET_8th_low 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>
#include <GxEPD2_7C.h>
#include <U8g2_for_Adafruit_GFX.h>

#include "GxEPD2_display_selection.h"
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

extern g_vars_t * g_vars_ptr;
extern g_config_t * g_config_ptr;

extern QueueHandle_t queueNotification;

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// TEMPLATE FUNCTIONS

/**
 * @brief Initializes the display screen template with IoT alarm branding and a dynamic label.
 *
 * This function initializes the screen display for the IoT alarm system, displaying a title, version, and a custom label
 * at the specified position on the screen. It uses the U8g2 library to render text on an OLED or similar screen.
 * 
 * The screen layout consists of:
 * - "IoT Alarm" centered at the top.
 * - "version 1.0" centered below the title.
 * - A dynamic label passed as an argument, centered below the version information.
 * 
 * @param label A string that represents the dynamic label to be displayed on the screen.
 *              This can be a message or a title for the current screen.
 * 
 * @details
 * - The title "IoT Alarm" is displayed using a specific font (`u8g2_font_maniac_tr`).
 * - The version "version 1.0" is displayed using a different font (`u8g2_font_courB10_tr`).
 * - The dynamic label is centered based on its width and displayed below the version.
 * - Text positioning is calculated to ensure proper centering on the screen, with a Y offset applied.
 * - The font sizes and styles are chosen to make the screen readable and aesthetically pleasing.
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * initScreenTemplate("System Ready");
 * @endcode
 */
void initScreenTemplate(const char * label);

/**
 * @brief Displays a menu screen with customizable options, selection highlight, and system status.
 *
 * This function sets up and updates the display for a menu screen on an IoT device, showing a main label,
 * multiple selectable options, a testing mode indicator (if applicable), current date and time, and system status 
 * icons (WiFi, GSM, and battery). It also highlights the selected option based on the provided selection ID.
 * 
 * @param label The title or label to be displayed at the top of the menu screen. This can be any string, such as a screen title or context label.
 * @param selection_id The ID of the currently selected option (0, 1, 2, or 3), used to highlight the chosen option.
 * @param test A boolean value indicating whether the device is in testing mode. If true, a "(testing mode)" label is displayed.
 * @param option1 The text for the first selectable option in the menu.
 * @param option2 The text for the second selectable option in the menu.
 * @param option3 The text for the third selectable option in the menu.
 * @param option4 The text for the fourth selectable option in the menu.
 * @param time A string representing the current time (e.g., "12:34").
 * @param date A string representing the current date (e.g., "2024-12-17").
 * @param wifi An integer representing the WiFi status (typically 0 for disconnected, 1 for connected).
 * @param gsm An integer representing the GSM status (typically 0 for disconnected, 1 for connected).
 * @param battery An integer representing the battery level (in percentage from 0 to 100).
 * 
 * @details
 * - The main label is displayed at the top of the screen with the font `u8g2_font_courB14_tr`.
 * - Four options are displayed below the main label, each option having its own line of text. These options are displayed using the font `u8g2_font_courB10_tr`.
 * - If the `test` flag is set to `true`, a "(testing mode)" label will appear in the upper-right corner of the screen, using the font `u8g2_font_courB08_tr`.
 * - The date and time are displayed using the `updateDatetime` function.
 * - The selection (highlighting of the chosen option) is handled by the `updateSelection` function, using the `selection_id` parameter.
 * - The system status icons for WiFi, GSM, and battery are updated using the `updateStatusIcons` function, which takes the respective status values as parameters.
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * menuScreenTemplate("Main Menu", 0, false, "Option 1", "Option 2", "Option 3", "Option 4", "12:34", "2024-12-17", 1, 0, 80);
 * @endcode
 * This will display:
 * - "Main Menu" as the screen label.
 * - "Option 1", "Option 2", "Option 3", and "Option 4" as selectable options.
 * - No "(testing mode)" label since `test` is `false`.
 * - Current time ("12:34") and date ("2024-12-17").
 * - WiFi status icon showing "connected", GSM status icon showing "disconnected", and battery status icon showing "80%" charge.
 */
void menuScreenTemplate(const char * label, int selection, bool test, const char * option1, const char * option2, const char * option3, const char * option4, const char * time, const char * date, int wifi, int gsm, int battery);

/**
 * @brief Displays an authentication screen with instructions, a PIN input field, the number of remaining attempts,
 *        and system status icons such as WiFi, GSM, and battery status.
 *
 * This function sets up and updates the display for an authentication screen on an IoT device. It includes a main
 * label, instructions for the user, a field to display the PIN being entered, the number of attempts remaining, 
 * an optional testing mode indicator (if applicable), and the current date and time. Additionally, it shows system 
 * status icons for WiFi, GSM, and battery.
 * 
 * @param label The title or label to be displayed at the top of the authentication screen.
 * @param test A boolean value indicating whether the device is in testing mode. If true, a "(testing mode)" label 
 *             is displayed in the top-right corner.
 * @param instructions1 The first line of instructions to guide the user through the authentication process.
 * @param instructions2 The second line of instructions to guide the user through the authentication process.
 * @param pin The PIN currently being entered by the user. This is displayed on the screen to show the user their input.
 * @param attempts The number of remaining authentication attempts. This value is displayed to inform the user how many attempts they have left.
 * @param time A string representing the current time (e.g., "12:34").
 * @param date A string representing the current date (e.g., "2024-12-17").
 * @param wifi An integer representing the WiFi status (typically 0 for disconnected, 1 for connected).
 * @param gsm An integer representing the GSM status (typically 0 for disconnected, 1 for connected).
 * @param battery An integer representing the battery level (in percentage from 0 to 100).
 * 
 * @details
 * - The main label is displayed at the top of the screen with the font `u8g2_font_courB14_tr`.
 * - Two lines of instructions are displayed to guide the user through the authentication process. These instructions
 *   are displayed using the font `u8g2_font_courB08_tr`.
 * - If the `test` flag is set to `true`, a "(testing mode)" label will appear in the upper-right corner of the screen, 
 *   using the font `u8g2_font_courB08_tr`.
 * - The PIN being entered by the user is displayed using the `updatePin` function.
 * - The number of remaining attempts is displayed using the `updateAttempts` function.
 * - The current date and time are displayed using the `updateDatetime` function.
 * - The status icons for WiFi, GSM, and battery are updated using the `updateStatusIcons` function, which takes 
 *   the respective status values as parameters.
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * authScreenTemplate("Authentication", false, "Enter your PIN", "to proceed", "****", 3, "12:34", "2024-12-17", 1, 0, 80);
 * @endcode
 * This will display:
 * - "Authentication" as the screen label.
 * - "Enter your PIN" and "to proceed" as instructions.
 * - The PIN field showing "****".
 * - Remaining attempts: 3.
 * - Current time ("12:34") and date ("2024-12-17").
 * - WiFi status icon showing "connected", GSM status icon showing "disconnected", and battery at 80%.
 */
void authScreenTemplate(const char * label, bool test, const char * instructions1, const char * instructions2, String pin, int attempts, const char * time, const char * date, int wifi, int gsm, int battery);

/**
 * @brief Displays an RFID screen with instructions, attempts, and system status icons such as WiFi, GSM, and battery status.
 *
 * This function sets up and updates the display for an RFID authentication screen on an IoT device. The screen includes a main 
 * label, instructions for the user, the number of attempts remaining, an optional testing mode indicator (if applicable),
 * and the current date and time. Additionally, it shows system status icons for WiFi, GSM, and battery.
 * 
 * @param label The title or label to be displayed at the top of the RFID screen.
 * @param test A boolean value indicating whether the device is in testing mode. If true, a "(testing mode)" label 
 *             is displayed in the top-right corner.
 * @param instructions1 The first line of instructions to guide the user through the RFID authentication process.
 * @param instructions2 The second line of instructions to guide the user through the RFID authentication process.
 * @param attempts The number of remaining authentication attempts. This value is displayed to inform the user how many attempts they have left.
 * @param time A string representing the current time (e.g., "12:34").
 * @param date A string representing the current date (e.g., "2024-12-17").
 * @param wifi An integer representing the WiFi status (typically 0 for disconnected, 1 for connected).
 * @param gsm An integer representing the GSM status (typically 0 for disconnected, 1 for connected).
 * @param battery An integer representing the battery level (in percentage from 0 to 100).
 * 
 * @details
 * - The main label is displayed at the top of the screen with the font `u8g2_font_courB14_tr`.
 * - Two lines of instructions are displayed to guide the user through the RFID authentication process. These instructions
 *   are displayed using the font `u8g2_font_courB08_tr`.
 * - The number of remaining attempts is displayed with a larger font (`u8g2_font_courB10_tr`), below the instructions.
 * - If the `test` flag is set to `true`, a "(testing mode)" label will appear in the upper-right corner of the screen, 
 *   using the font `u8g2_font_courB08_tr`.
 * - The current date and time are displayed using the `updateDatetime` function.
 * - The status icons for WiFi, GSM, and battery are updated using the `updateStatusIcons` function, which takes 
 *   the respective status values as parameters.
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * rfidScreenTemplate("RFID Authentication", false, "Scan your RFID tag", "to proceed", 3, "12:34", "2024-12-17", 1, 0, 80);
 * @endcode
 * This will display:
 * - "RFID Authentication" as the screen label.
 * - "Scan your RFID tag" and "to proceed" as instructions.
 * - Remaining attempts: 3.
 * - Current time ("12:34") and date ("2024-12-17").
 * - WiFi status icon showing "connected", GSM status icon showing "disconnected", and battery at 80%.
 */
void rfidScreenTemplate(const char * label, bool test, const char * instructions1, const char * instructions2, int attempts, const char * time, const char * date, int wifi, int gsm, int battery);

/**
 * @brief Displays an alarm screen with event data, system status, and user input fields.
 *
 * This function sets up and updates the display for an alarm screen on an IoT device. The screen includes a main label, 
 * event data or countdown information, status updates, the number of remaining authentication attempts, 
 * an optional testing mode indicator, and the current date and time. Additionally, it shows system status icons 
 * for WiFi, GSM, and battery.
 * 
 * @param label The title or label to be displayed at the top of the alarm screen.
 * @param test A boolean value indicating whether the device is in testing mode. If true, a "(testing mode)" label 
 *             is displayed in the top-right corner.
 * @param status A string representing the current status of the alarm (e.g., "Active", "Inactive").
 * @param data A string representing the event type or countdown (e.g., "Time Remaining", "Last Triggered").
 * @param data_load The current value associated with the event or countdown (e.g., time remaining in seconds).
 * @param pin The current PIN input by the user, displayed to verify PIN authentication.
 * @param attempts The number of remaining authentication attempts. This value is displayed to inform the user how many attempts they have left.
 * @param time A string representing the current time (e.g., "12:34").
 * @param date A string representing the current date (e.g., "2024-12-17").
 * @param wifi An integer representing the WiFi status (typically 0 for disconnected, 1 for connected).
 * @param gsm An integer representing the GSM status (typically 0 for disconnected, 1 for connected).
 * @param battery An integer representing the battery level (in percentage from 0 to 100).
 * 
 * @details
 * - The main label is displayed at the top of the screen with the font `u8g2_font_courB14_tr`.
 * - Event or countdown data is displayed below the label, followed by the current status of the alarm (e.g., "Active").
 * - The number of remaining attempts is displayed below the PIN input field.
 * - If the `test` flag is set to `true`, a "(testing mode)" label will appear in the upper-right corner of the screen, 
 *   using the font `u8g2_font_courB08_tr`.
 * - The current date and time are displayed using the `updateDatetime` function.
 * - The status icons for WiFi, GSM, and battery are updated using the `updateStatusIcons` function, which takes 
 *   the respective status values as parameters.
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * alarmScreenTemplate("Alarm System", false, "Inactive", "Countdown: 30s", "1234", 3, 30, "12:34", "2024-12-17", 1, 0, 80);
 * @endcode
 * This will display:
 * - "Alarm System" as the screen label.
 * - "Countdown: 30s" as the event type and countdown data.
 * - "Inactive" as the current status of the alarm.
 * - Remaining attempts: 3.
 * - Current time ("12:34") and date ("2024-12-17").
 * - WiFi status icon showing "connected", GSM status icon showing "disconnected", and battery at 80%.
 */
void alarmScreenTemplate(const char * label, bool test, const char * status, const char * data, String pin, int attempts, int data_load, const char * time, const char * date, int wifi, int gsm, int battery);

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// UPDATE FUNCTIONS

/**
 * @brief Updates the status icons on the screen for WiFi, GSM, and battery levels.
 * 
 * This function displays icons representing the current WiFi signal strength, GSM signal strength, 
 * and battery level on the screen. The icons are selected based on the provided signal and battery levels 
 * and are displayed at predefined positions on the screen.
 * 
 * @param wifi The current WiFi signal strength in dBm. 
 *             - A value greater than 0 indicates a disconnected state.
 *             - Values between -60 and -85 represent varying WiFi signal strengths.
 * @param gsm The current GSM signal strength in dBm.
 *             - The value ranges from 0 (no signal) to 31 (maximum signal).
 *             - A value of 99 indicates no GSM signal or unknown status.
 * @param battery The current battery level as a percentage (0-100).
 *               - The value is used to select an appropriate icon for battery level.
 * 
 * @details
 * - WiFi: The function displays one of several WiFi icons based on the WiFi signal strength.
 *         - `< \ue217 >`: No signal
 *         - `< \ue21a >`: Excellent signal
 *         - `< \ue219 >`: Good signal
 *         - `< \ue218 >`: Fair signal
 *         - `< \ue217 >`: Poor signal
 * 
 * - Battery: The function displays one of several battery icons depending on the battery percentage.
 *           - Icons range from empty (`< \ue24c >`) to full (`< \ue254 >`).
 * 
 * - GSM: The function displays one of several GSM signal strength icons.
 *        - `< \ue258 >`: No signal
 *        - `< \ue259 >`: Low signal
 *        - `< \ue25a >`: Medium signal
 *        - `< \ue25b >`: High signal
 *        - `< \ue25c >`: Full signal
 * 
 * The icons are drawn at specific positions (WiFi at `(232, 16+Y_OFFSET)`, GSM at `(202, 16+Y_OFFSET)`, 
 * and battery at `(217, 16+Y_OFFSET)`) on the screen using a specific font (`u8g2_font_siji_t_6x10`).
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * updateStatusIcons(-65, 20, 80);
 * @endcode
 * This will display:
 * - A good WiFi signal (icon `< \ue219 >`).
 * - A medium GSM signal (icon `< \ue25a >`).
 * - A battery level of 80% (icon `< \ue251 >`).
 */
void updateStatusIcons(int wifi, int gsm, int battery);

/**
 * @brief Updates the date and time on the display.
 * 
 * This function displays the current date and time on the screen at predefined positions.
 * The date is shown at one position, and the time is displayed at another, formatted 
 * as provided in the function arguments.
 * 
 * @param date The current date as a string, typically in the format "YYYY-MM-DD" or similar.
 * @param time The current time as a string, typically in the format "HH:MM:SS" or similar.
 * 
 * @details
 * The function uses the `u8g2_font_courB08_tr` font to display the date and time on the screen.
 * - The date is displayed at the position `(185, 115 + Y_OFFSET)`.
 * - The time is displayed at the position `(215, 101 + Y_OFFSET)`.
 * 
 * The Y-offset is used to adjust the vertical positioning, allowing for alignment 
 * with other content on the screen.
 * 
 * The date and time are drawn with the provided font, making them readable on 
 * the display.
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * updateDatetime("2024-12-17", "14:30:00");
 * @endcode
 * This will display:
 * - Date: "2024-12-17" at the position `(185, 115 + Y_OFFSET)`.
 * - Time: "14:30:00" at the position `(215, 101 + Y_OFFSET)`.
 */
void updateDatetime(const char * date, const char * time);

/**
 * @brief Updates the PIN on the display with a masked version.
 * 
 * This function takes a PIN string, removes the '#' delimiter if present, and then displays
 * a masked version of the PIN on the screen. The masked version of the PIN replaces all 
 * characters with the `#` symbol, except for the delimiter, if it's present in the input.
 * 
 * @param pin The PIN string, which may include a `#` delimiter. 
 *            The delimiter is used to separate the actual PIN from a possible
 *            confirmation character (e.g., PIN#).
 * @param x The x-coordinate where the PIN should be displayed on the screen.
 * @param y The y-coordinate where the PIN should be displayed on the screen.
 * 
 * @details
 * - If the `pin` string contains a delimiter (`#`), the function splits the string 
 *   at the delimiter and only considers the part after it.
 * - It then constructs a masked PIN by replacing all characters with `#` and displays
 *   the result on the screen.
 * - The PIN is displayed using the `u8g2_font_courB18_tr` font at the specified `(x, y)` coordinates.
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * updatePin("1234#", 20, 50);
 * @endcode
 * This will display:
 * - PIN: #### at the position `(20, 50)` on the screen.
 */
void updatePin(String pin, int x, int y);

/**
 * @brief Updates the display with the number of attempts.
 * 
 * This function displays the number of remaining attempts on the screen. It uses 
 * a specified font and coordinates to print the string "attempts: X", where X 
 * is the number of attempts left.
 * 
 * @param attempts The number of remaining attempts to be displayed.
 * @param x The x-coordinate where the attempts count should be displayed on the screen.
 * @param y The y-coordinate where the attempts count should be displayed on the screen.
 * 
 * @details
 * - The function uses the `u8g2_font_courB10_tr` font to display the attempt count.
 * - It uses the specified `(x, y)` coordinates to position the text on the screen.
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * updateAttempts(3, 20, 50);
 * @endcode
 * This will display:
 * - "attempts: 3" at the position `(20, 50)` on the screen.
 */
void updateAttempts(int attempts, int x, int y);

/**
 * @brief Updates the display to show a selection marker (">") for the current selected item.
 * 
 * This function displays a ">" marker next to the currently selected item in a list or menu. The 
 * marker moves based on the `selection_id`, highlighting the corresponding menu item. If the 
 * `selection_id` is negative, it displays the left arrow ("<") instead.
 * 
 * @param selection_id The index of the selected item. A negative value indicates that a left 
 *                     arrow ("<") should be displayed, while non-negative values highlight a 
 *                     specific menu item with a ">" marker.
 * 
 * @details
 * - The function uses the `u8g2_font_courB10_tr` font for the selection marker.
 * - Based on the `selection_id`, it sets the cursor at different positions to display the ">" marker.
 * - For negative values of `selection_id`, a "<" marker is shown at a fixed position.
 * - The `selection_id` determines the vertical position of the ">" marker, with different positions 
 *   for values 0, 1, 2, and 3.
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * updateSelection(2);
 * @endcode
 * This will display the ">" marker at the vertical position corresponding to the third menu item.
 */
void updateSelection(int selection_id);

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// HELPER FUNCTIONS

/**
 * @brief Returns a selection ID based on the current state and selection.
 * 
 * This function maps a given `state` and `selection` to a specific integer ID. 
 * The ID is used to identify the selection for the given state, allowing for 
 * easier state transitions and decision-making in the application. 
 * It provides a mapping of states to their corresponding selections and returns 
 * an integer value that is used to determine the action in the system.
 * 
 * @param state The current state of the system, represented by an enumeration `States`.
 * @param selection The selected option, represented by an integer corresponding 
 *                 to the possible selections for that state.
 * 
 * @return An integer ID corresponding to the selection in the current state. 
 *         If no valid selection is found, the function returns a special error code:
 *         - `-2`: Invalid selection in the given state.
 *         - `-1`: Return selection (indicating no action).
 *         - `-3`: Invalid state.
 * 
 * @details
 * This function handles several states of the system, each with its own set of possible selections:
 * - **STATE_INIT**: Handles initial setup actions like setup, alarm, test, or reboot.
 * - **STATE_SETUP**: Handles various setup actions, such as starting the Wi-Fi station (STA) mode, 
 *                   opening/closing Zigbee, adding/removing RFID, or resetting the Zigbee network.
 * - **STATE_ALARM_IDLE**: Handles alarm-related actions such as locking, changing the password, 
 *                        or rebooting the system.
 * - **STATE_TEST_IDLE**: Handles test-related actions similar to `STATE_ALARM_IDLE`.
 * 
 * Example usage:
 * @code
 * int selectionId = getSelectionId(STATE_SETUP, SELECTION_SETUP_ADD_RFID);
 * @endcode
 * In this case, it will return `2`, corresponding to the `SELECTION_SETUP_ADD_RFID`.
 * 
 * @see States, SELECTION_INIT_SETUP, SELECTION_SETUP_START_STA, etc.
 */
int getSelectionId(States state, int selection);

/**
 * @brief Waits for the E Ink display to be ready.
 * 
 * This function checks the `EPD_BUSY` pin to determine whether the E Ink display 
 * is busy performing an operation (e.g., refreshing or updating the screen). 
 * The function continuously checks the pin until the display is no longer busy, 
 * then it returns and the program can continue with the next operation.
 * 
 * It utilizes a non-blocking delay using `vTaskDelay` from FreeRTOS to prevent 
 * blocking the main loop while waiting for the display to become ready.
 * 
 * @details
 * The `EPD_BUSY` pin typically goes high when the E Ink display is busy and 
 * low when it is ready for the next operation. The function periodically checks 
 * the pin and waits for the display to be ready.
 * 
 * Example usage:
 * @code
 * waitReady();  // Wait until the E Ink display is ready.
 * @endcode
 */
void waitReady();

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------

void initEink() {
    Serial.printf("EINK display initialisation\n");
    pinMode(EPD_BUSY, INPUT_PULLDOWN);

    // set up communication
    SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI);
    display.init(115200, true, 2, false, SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    u8g2Fonts.begin(display);

    if (display.pages() > 1) {
        Serial.print("Eink display: pages = ");
        Serial.print(display.pages());
        Serial.print(" page height = ");
        Serial.println(display.pageHeight());
    }

    // set default settings
    display.setRotation(1);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(1);
    display.setPartialWindow(0, 0, display.width(), display.height());
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

    // show init screen
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawRect(0, Y_OFFSET, display.width(), display.height()-Y_OFFSET, GxEPD_BLACK);
        initScreenTemplate("Petr Zerzan");
    } while (display.nextPage());
}

void displayNotification(notificationScreenId id, int param, int duration) {
    notification_t * notification = (notification_t *)malloc(sizeof(notification_t));
    if (notification == NULL) {
        esplogW(TAG_LIB_DISPLAY, "(displayNotification)", "Failed to allocate memory for notification queue!");
        return;
    }

    notification->id = id;
    notification->param = param;
    notification->duration = duration;

    if (xQueueSend(queueNotification, &notification, pdMS_TO_TICKS(10)) != pdPASS) {
        esplogW(TAG_LIB_DISPLAY, "(displayNotification)", "Failed to send notification to queue!");
        free(notification);
        return;
    } else {
        esplogI(TAG_LIB_DISPLAY, "(displayNotification)", "Notification has been enqueued! (id: %d)", *notification);
    }
}

void displayNotificationHandler(notificationScreenId notification, int param) {
    char string[48];
    switch (notification) {
        case NOTIFICATION_AUTH_CHECK_SUCCESS:
            notificationScreenTemplate("Correct PIN", "Access permited!");
            break;
        case NOTIFICATION_AUTH_CHECK_ERROR:
            notificationScreenTemplate("Wrong PIN", "Access denied!");
            break;
        case NOTIFICATION_AUTH_SET_SUCCESS:
            notificationScreenTemplate("PIN set", "New PIN was set!");
            break;
        case NOTIFICATION_AUTH_SET_ERROR:
            notificationScreenTemplate("PIN error", "PIN set failed!");
            break;
        case NOTIFICATION_RFID_CHECK_SUCCESS:
            notificationScreenTemplate("Correct RFID", "RFID card recognised!");
            break;
        case NOTIFICATION_RFID_CHECK_ERROR:
            notificationScreenTemplate("Wrong RFID", "RFID card not recognised!");
            break;
        case NOTIFICATION_RFID_ADD_SUCCESS:
            notificationScreenTemplate("RFID added", "RFID card added!");
            break;
        case NOTIFICATION_RFID_ADD_ERROR:
            notificationScreenTemplate("RFID add error", "RFID card add failed!");
            break;
        case NOTIFICATION_RFID_DEL_SUCCESS:
            notificationScreenTemplate("RFID deleted", "RFID card deleted!");
            break;
        case NOTIFICATION_RFID_DEL_ERROR:
            notificationScreenTemplate("RFID delete error", "RFID card delete failed!");
            break;
        case NOTIFICATION_ZIGBEE_NET_OPEN:
            sprintf(string, "network joining is now open for %d seconds!", param);
            notificationScreenTemplate("ZIGBEE open", string);
            break;
        case NOTIFICATION_ZIGBEE_NET_CLOSE:
            notificationScreenTemplate("ZIGBEE closed", "network joining is now closed!");
            break;
        case NOTIFICATION_ZIGBEE_NET_CLEAR:
            notificationScreenTemplate("ZIGBEE cleared", "network has been cleaned!");
            break;
        case NOTIFICATION_ZIGBEE_NET_RESET:
            break;
        case NOTIFICATION_ZIGBEE_ATTR_REPORT:
            notificationScreenTemplate("ZIGBEE report", "alarm event has been triggered!");
            break;
        case NOTIFICATION_ZIGBEE_DEV_ANNCE:
            notificationScreenTemplate("ZIGBEE join", "zigbee device has joined network!");
            break;
        case NOTIFICATION_ZIGBEE_DEV_LEAVE:
            notificationScreenTemplate("ZIGBEE leave", "zigbee device has leaved network!");
            break;
        case NOTIFICATION_ZIGBEE_DEV_COUNT:
            sprintf(string, "%d devices are connected!", param);
            notificationScreenTemplate("ZIGBEE count", string);
            break;
        case NOTIFICATION_MQTT_CONNECTED:
            notificationScreenTemplate("MQTT connected", "MQTT server has been connected successfully!");
            break;
        case NOTIFICATION_MQTT_DISCONECTED:
            notificationScreenTemplate("MQTT disconnected", "MQTT server connection failed!");
            break;
        case NOTIFICATION_WIFI_CONNEDTED:
            notificationScreenTemplate("WiFi connected", "WiFi connection has been established!");
            break;
        case NOTIFICATION_WIFI_DISCONECTED:
            notificationScreenTemplate("WiFi disconnected", "WiFi connection failed!");
            break;
        default:
            break;
    }

    waitReady();
}

void displayRestart() {
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawRect(0, Y_OFFSET, display.width(), display.height()-Y_OFFSET, GxEPD_BLACK);
        initScreenTemplate("Rebooting...");
    } while (display.nextPage());
}

void displayLoad() {
    if (g_vars_ptr->refresh_display.refresh) {
        g_vars_ptr->refresh_display.refresh = false;

        display.setPartialWindow(0, 0, display.width(), display.height());
        display.firstPage();
        do {
            display.fillScreen(GxEPD_WHITE);

            // display border rectangle
            display.drawRect(0, Y_OFFSET, display.width(), display.height()-Y_OFFSET, GxEPD_BLACK); // <- my screen has obviously different height than class expects

            int selection_id = getSelectionId(g_vars_ptr->state, g_vars_ptr->selection);
            switch (g_vars_ptr->state) {
                case STATE_INIT:
                    menuScreenTemplate(getStateText(g_vars_ptr->state, true), selection_id, false, "setup", "alarm", "test mode", "reboot", g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_SETUP:
                    menuScreenTemplate(getStateText(g_vars_ptr->state, true), selection_id, false, "WiFi setup", "ZIGBEE setup", "RFID setup", "hard reset", g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_SETUP_AP:
                    initScreenTemplate("WiFi AP is now active...");
                    break;

                case STATE_SETUP_HARD_RESET:
                    initScreenTemplate("Please confirm hard reset...");
                    break;

                case STATE_SETUP_RFID_ADD:
                case STATE_SETUP_RFID_DEL:
                case STATE_SETUP_RFID_CHECK:
                    rfidScreenTemplate(getStateText(g_vars_ptr->state, true), false, "Please, insert RFID card:", "", g_vars_ptr->attempts, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_IDLE:
                    menuScreenTemplate(getStateText(g_vars_ptr->state, true), selection_id, false, "lock", "PIN setup", "reboot", "", g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_TEST_IDLE:
                    menuScreenTemplate(getStateText(g_vars_ptr->state, true), selection_id, true, "lock", "PIN setup", "reboot", "", g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_OK:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), false, "status: OK", "events", g_vars_ptr->pin.c_str(), g_vars_ptr->attempts, g_vars_ptr->alarm.alarm_events, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_TEST_OK:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), true, "status: OK", "events", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->alarm.alarm_events, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_C:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), false, "status: STARTING", "remaining", g_vars_ptr->pin, g_vars_ptr->attempts, (g_config_ptr->alarm_countdown_s*1000-g_vars_ptr->time_temp)/1000, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_TEST_C:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), true, "status: STARTING", "remaining", g_vars_ptr->pin, g_vars_ptr->attempts, (g_config_ptr->alarm_countdown_s*1000-g_vars_ptr->time_temp)/1000, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_W:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), false, "status: WARNING", "remaining", g_vars_ptr->pin, g_vars_ptr->attempts, (g_config_ptr->alarm_e_countdown_s*1000-g_vars_ptr->time_temp)/1000, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_TEST_W:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), true, "status: WARNING", "remaining", g_vars_ptr->pin, g_vars_ptr->attempts, (g_config_ptr->alarm_e_countdown_s*1000-g_vars_ptr->time_temp)/1000, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_E:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), false, "status: EMERGENCY", "events", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->alarm.alarm_events, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_TEST_E:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), true, "status: EMERGENCY", "events", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->alarm.alarm_events, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_SETUP_HARD_RESET_ENTER_PIN:
                case STATE_SETUP_AP_ENTER_PIN:
                case STATE_SETUP_RFID_ADD_ENTER_PIN:
                case STATE_SETUP_RFID_DEL_ENTER_PIN:
                case STATE_ALARM_LOCK_ENTER_PIN:
                case STATE_TEST_LOCK_ENTER_PIN:
                case STATE_ALARM_UNLOCK_ENTER_PIN:
                case STATE_TEST_UNLOCK_ENTER_PIN:
                case STATE_ALARM_CHANGE_ENTER_PIN1:
                case STATE_TEST_CHANGE_ENTER_PIN1:
                case STATE_SETUP_PIN1:
                    authScreenTemplate(getStateText(g_vars_ptr->state, true), false, "Please, type in PIN code,", "or use RFID card:", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_CHANGE_ENTER_PIN2:
                case STATE_TEST_CHANGE_ENTER_PIN2:
                case STATE_SETUP_PIN2:
                    authScreenTemplate(getStateText(g_vars_ptr->state, true), false, "Please, type in new PIN code:", "", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_CHANGE_ENTER_PIN3:
                case STATE_TEST_CHANGE_ENTER_PIN3:
                case STATE_SETUP_PIN3:
                    authScreenTemplate(getStateText(g_vars_ptr->state, true), false, "Please, repeat previously", "set PIN code:", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                default:
                    // lcd.print("State was not recognised!");
                    esplogW(TAG_LIB_DISPLAY, "(loadScreen)", "Unrecognised state for loading display data!\n");
                    break;
            }

        } while (display.nextPage());
    }

    else {
        String data;
        int x, y;

        if (g_vars_ptr->refresh_display.refresh_selection) {
            g_vars_ptr->refresh_display.refresh_selection = false;

            // selection
            display.setPartialWindow(10, 32+Y_OFFSET_8th_high, 20, 80+Y_OFFSET_8th_low);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                updateSelection(getSelectionId(g_vars_ptr->state, g_vars_ptr->selection));
            } while (display.nextPage());

            // special selections for state STATE_SETUP
            if (g_vars_ptr->state == STATE_SETUP) {
                display.setPartialWindow(30, 48+Y_OFFSET_8th_high, 150, 40+Y_OFFSET_8th_low);
                display.firstPage();
                do {
                    display.fillScreen(GxEPD_WHITE);
                    u8g2Fonts.setFont(u8g2_font_courB10_tr);

                    u8g2Fonts.setCursor(36, 65+Y_OFFSET);
                    switch (g_vars_ptr->selection) {
                        case SELECTION_SETUP_OPEN_ZB:
                            u8g2Fonts.print("ZIGBEE open");
                            break;
                        case SELECTION_SETUP_CLOSE_ZB:
                            u8g2Fonts.print("ZIGBEE close");
                            break;
                        case SELECTION_SETUP_CLEAR_ZB:
                            u8g2Fonts.print("ZIGBEE clear");
                            break;
                        case SELECTION_SETUP_RESET_ZB:
                            u8g2Fonts.print("ZIGBEE reset");
                            break;
                        
                        default:
                            u8g2Fonts.print("ZIGBEE setup");
                            break;
                    }


                    u8g2Fonts.setCursor(36, 83+Y_OFFSET);
                    switch (g_vars_ptr->selection) {
                        case SELECTION_SETUP_ADD_RFID:
                            u8g2Fonts.print("RFID add");
                            break;
                        case SELECTION_SETUP_DEL_RFID:
                            u8g2Fonts.print("RFID remove");
                            break;
                        case SELECTION_SETUP_CHECK_RFID:
                            u8g2Fonts.print("RFID check");
                            break;
                        
                        default:
                            u8g2Fonts.print("RFID setup");
                            break;
                    }
                } while (display.nextPage());
            }
        }

        if (g_vars_ptr->refresh_display.refresh_status) {
            g_vars_ptr->refresh_display.refresh_status = false;

            // status icons
            display.setPartialWindow(200, 8+Y_OFFSET_8th_low, 44, 16+Y_OFFSET_8th_low);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                updateStatusIcons(g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_datetime) {
            g_vars_ptr->refresh_display.refresh_datetime = false;
            // datetime
            display.setPartialWindow(180, 88+Y_OFFSET_8th_low, 64, 32+Y_OFFSET_8th_low);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                updateDatetime(g_vars_ptr->date.c_str(), g_vars_ptr->time.c_str());
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_pin) {
            g_vars_ptr->refresh_display.refresh_pin = false;

            // pin
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                switch (g_vars_ptr->state) {
                    case STATE_SETUP_HARD_RESET_ENTER_PIN:
                    case STATE_SETUP_AP_ENTER_PIN:
                    case STATE_SETUP_RFID_ADD_ENTER_PIN:
                    case STATE_SETUP_RFID_DEL_ENTER_PIN:
                    case STATE_ALARM_LOCK_ENTER_PIN:
                    case STATE_TEST_LOCK_ENTER_PIN:
                    case STATE_ALARM_UNLOCK_ENTER_PIN:
                    case STATE_TEST_UNLOCK_ENTER_PIN:
                    case STATE_ALARM_CHANGE_ENTER_PIN1:
                    case STATE_TEST_CHANGE_ENTER_PIN1:
                    case STATE_SETUP_PIN1:
                    case STATE_ALARM_CHANGE_ENTER_PIN2:
                    case STATE_TEST_CHANGE_ENTER_PIN2:
                    case STATE_SETUP_PIN2:
                    case STATE_ALARM_CHANGE_ENTER_PIN3:
                    case STATE_TEST_CHANGE_ENTER_PIN3:
                    case STATE_SETUP_PIN3:
                        display.setPartialWindow(20, 64+Y_OFFSET_8th_low, 180, 24+Y_OFFSET_8th_high);
                        x = 20; y = 82+Y_OFFSET;
                        break;

                    case STATE_ALARM_OK:
                    case STATE_TEST_OK:
                    case STATE_ALARM_C:
                    case STATE_TEST_C:
                    case STATE_ALARM_W:
                    case STATE_TEST_W:
                    case STATE_ALARM_E:
                    case STATE_TEST_E:
                        display.setPartialWindow(20, 72+Y_OFFSET_8th_low, 180, 24+Y_OFFSET_8th_high);
                        x = 20; y = 94+Y_OFFSET;
                        break;
                    
                    default:
                        return;
                }
                updatePin(g_vars_ptr->pin, x, y);
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_attempts) {
            g_vars_ptr->refresh_display.refresh_attempts = false;

            // attempts
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                switch (g_vars_ptr->state) {
                    case STATE_SETUP_HARD_RESET_ENTER_PIN:
                    case STATE_SETUP_AP_ENTER_PIN:
                    case STATE_SETUP_RFID_ADD_ENTER_PIN:
                    case STATE_SETUP_RFID_DEL_ENTER_PIN:
                    case STATE_ALARM_LOCK_ENTER_PIN:
                    case STATE_TEST_LOCK_ENTER_PIN:
                    case STATE_ALARM_UNLOCK_ENTER_PIN:
                    case STATE_TEST_UNLOCK_ENTER_PIN:
                    case STATE_ALARM_CHANGE_ENTER_PIN1:
                    case STATE_TEST_CHANGE_ENTER_PIN1:
                    case STATE_SETUP_PIN1:
                    case STATE_ALARM_CHANGE_ENTER_PIN2:
                    case STATE_TEST_CHANGE_ENTER_PIN2:
                    case STATE_SETUP_PIN2:
                    case STATE_ALARM_CHANGE_ENTER_PIN3:
                    case STATE_TEST_CHANGE_ENTER_PIN3:
                    case STATE_SETUP_PIN3:
                        display.setPartialWindow(20, 96+Y_OFFSET_8th_low, 130, 16+Y_OFFSET_8th_high);
                        x = 20; y = 102+Y_OFFSET;
                        break;

                    case STATE_ALARM_OK:
                    case STATE_TEST_OK:
                    case STATE_ALARM_C:
                    case STATE_TEST_C:
                    case STATE_ALARM_W:
                    case STATE_TEST_W:
                    case STATE_ALARM_E:
                    case STATE_TEST_E:
                        display.setPartialWindow(20, 104+Y_OFFSET_8th_low, 130, 16+Y_OFFSET_8th_high);
                        x = 20; y = 112+Y_OFFSET;
                        break;
                    
                    default:
                        return;
                }
                updateAttempts(g_vars_ptr->attempts, x, y);
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_countdown) {
            g_vars_ptr->refresh_display.refresh_countdown = false;

            // events / countdown
            display.setPartialWindow(20, 32+Y_OFFSET_8th_low, 140, 16+Y_OFFSET_8th_high);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                u8g2Fonts.setFont(u8g2_font_courB10_tr);
                u8g2Fonts.setCursor(20, 42+Y_OFFSET);
                switch (g_vars_ptr->state) {
                    case STATE_ALARM_C:
                    case STATE_TEST_C:
                        data = String(String("remaining: ") + String((g_config_ptr->alarm_countdown_s*1000-g_vars_ptr->time_temp)/1000));
                        break;

                    case STATE_ALARM_W:
                    case STATE_TEST_W:
                        data = String(String("remaining: ") + String((g_config_ptr->alarm_e_countdown_s*1000-g_vars_ptr->time_temp)/1000));
                        break;

                    default:
                        return;
                }
                u8g2Fonts.print(data);
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_events) {
            g_vars_ptr->refresh_display.refresh_events = false;

            // events / countdown
            display.setPartialWindow(20, 32+Y_OFFSET_8th_low, 140, 16+Y_OFFSET_8th_high);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                u8g2Fonts.setFont(u8g2_font_courB10_tr);
                u8g2Fonts.setCursor(20, 42+Y_OFFSET);
                data = String(String("events: ") + String(g_vars_ptr->alarm.alarm_events));
                u8g2Fonts.print(data);
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_alarm_status) {
            g_vars_ptr->refresh_display.refresh_alarm_status = false;

            // alarm status
            display.setPartialWindow(20, 56+Y_OFFSET_8th_low, 140, 16+Y_OFFSET_8th_high);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                u8g2Fonts.setFont(u8g2_font_courB10_tr);
                u8g2Fonts.setCursor(20, 60+Y_OFFSET);
                switch (g_vars_ptr->state) {
                    case STATE_ALARM_OK:
                    case STATE_TEST_OK:
                        data = String("status: OK");
                        break;

                    case STATE_ALARM_C:
                    case STATE_TEST_C:
                        data = String("status: STARTING");

                    case STATE_ALARM_W:
                    case STATE_TEST_W:
                        data = String("status: WARNING");

                    case STATE_ALARM_E:
                    case STATE_TEST_E:
                        data = String("status: EMERGENCY");
                        break;

                    default:
                        return;
                }
                u8g2Fonts.print(data);
            } while (display.nextPage());
        }   
    }

    waitReady();
}

// ---------------------------------------------------------------------------
// TEMPLATE FUNCTIONS

void menuScreenTemplate(const char * label, int selection_id, bool test, const char * option1, const char * option2, const char * option3, const char * option4, const char * time, const char * date, int wifi, int gsm, int battery) {
    uint16_t x, y, w, h;
    int16_t tx, ty, tw, th;

    // main label
    u8g2Fonts.setFont(u8g2_font_courB14_tr);
    u8g2Fonts.setCursor(7, 18+Y_OFFSET);
    u8g2Fonts.print(label);
    display.drawFastHLine(5, 25+Y_OFFSET, display.width() - 10, GxEPD_BLACK);

    // options
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    u8g2Fonts.setCursor(36, 47+Y_OFFSET);
    u8g2Fonts.print(option1);
    u8g2Fonts.setCursor(36, 65+Y_OFFSET);
    u8g2Fonts.print(option2);
    u8g2Fonts.setCursor(36, 83+Y_OFFSET);
    u8g2Fonts.print(option3);
    u8g2Fonts.setCursor(36, 101+Y_OFFSET);
    u8g2Fonts.print(option4);

    // testing mode label
    if (test) {
        u8g2Fonts.setFont(u8g2_font_courB08_tr);
        u8g2Fonts.setCursor(163, 36+Y_OFFSET);
        u8g2Fonts.print("(testing mode)");   
    }

    // date & time
    updateDatetime(date, time);

    // selection
    updateSelection(selection_id);

    // status icons
    updateStatusIcons(wifi, gsm, battery);
}

void rfidScreenTemplate(const char * label, bool test, const char * instructions1, const char * instructions2, int attempts, const char * time, const char * date, int wifi, int gsm, int battery) {
    uint16_t x, y, w, h; 
    int16_t tx, ty, tw, th;

    // main label
    u8g2Fonts.setFont(u8g2_font_courB14_tr);
    u8g2Fonts.setCursor(5, 18+Y_OFFSET);
    u8g2Fonts.print(label);
    display.drawFastHLine(5, 25+Y_OFFSET, display.width() - 10, GxEPD_BLACK);

    // instructions
    u8g2Fonts.setFont(u8g2_font_courB08_tr);
    u8g2Fonts.setCursor(7, 36+Y_OFFSET);
    u8g2Fonts.print(instructions1);
    u8g2Fonts.setFont(u8g2_font_courB08_tr);
    u8g2Fonts.setCursor(7, 48+Y_OFFSET);
    u8g2Fonts.print(instructions2);

    // attempts
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    u8g2Fonts.setCursor(20, 102+Y_OFFSET);
    u8g2Fonts.printf("attempts: %d", attempts);
    
    // testing mode label
    if (test) {
        u8g2Fonts.setFont(u8g2_font_courB08_tr);
        u8g2Fonts.setCursor(163, 36+Y_OFFSET);
        u8g2Fonts.print("(testing mode)");
    }

    // date & time
    updateDatetime(date, time);

    // status icons
    updateStatusIcons(wifi, gsm, battery);
}

void authScreenTemplate(const char * label, bool test, const char * instructions1, const char * instructions2, String pin, int attempts, const char * time, const char * date, int wifi, int gsm, int battery) {
    uint16_t x, y, w, h; 
    int16_t tx, ty, tw, th;

    // main label
    u8g2Fonts.setFont(u8g2_font_courB14_tr);
    u8g2Fonts.setCursor(5, 18+Y_OFFSET);
    u8g2Fonts.print(label);
    display.drawFastHLine(5, 25+Y_OFFSET, display.width() - 10, GxEPD_BLACK);

    // instructions
    u8g2Fonts.setFont(u8g2_font_courB08_tr);
    u8g2Fonts.setCursor(7, 36+Y_OFFSET);
    u8g2Fonts.print(instructions1);
    u8g2Fonts.setFont(u8g2_font_courB08_tr);
    u8g2Fonts.setCursor(7, 48+Y_OFFSET);
    u8g2Fonts.print(instructions2);
    
    // testing mode label
    if (test) {
        u8g2Fonts.setFont(u8g2_font_courB08_tr);
        u8g2Fonts.setCursor(163, 36+Y_OFFSET);
        u8g2Fonts.print("(testing mode)");
    }

    // pin
    updatePin(pin, 20, 82+Y_OFFSET);

    // attempts
    updateAttempts(attempts, 20, 102+Y_OFFSET);

    // date & time
    updateDatetime(date, time);

    // status icons
    updateStatusIcons(wifi, gsm, battery);
}

void alarmScreenTemplate(const char * label, bool test, const char * status, const char * data, String pin, int attempts, int data_load, const char * time, const char * date, int wifi, int gsm, int battery) {
    uint16_t x, y, w, h; 
    int16_t tx, ty, tw, th;

    // main label
    u8g2Fonts.setFont(u8g2_font_courB14_tr);
    u8g2Fonts.setCursor(7, 18+Y_OFFSET);
    u8g2Fonts.print(label);
    display.drawFastHLine(5, 25+Y_OFFSET, display.width() - 10, GxEPD_BLACK);

    // data (events / countdown)
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    u8g2Fonts.setCursor(20, 42+Y_OFFSET);
    u8g2Fonts.printf("%s: %d", data, data_load);
    u8g2Fonts.setCursor(20, 60+Y_OFFSET);
    u8g2Fonts.print(status);

    // testing mode label
    if (test) {
        u8g2Fonts.setFont(u8g2_font_courB08_tr);
        u8g2Fonts.setCursor(163, 36+Y_OFFSET);
        u8g2Fonts.print("(testing mode)");   
    }

    // attempts
    updateAttempts(attempts, 20, 112+Y_OFFSET);

    // pin
    updatePin(pin, 20, 94+Y_OFFSET);

    // date & time
    updateDatetime(date, time);

    // status icons
    updateStatusIcons(wifi, gsm, battery);
}

void initScreenTemplate(const char * label) {
    uint16_t x, y, w, h;
    int16_t tx, ty, tw, th;

    u8g2Fonts.setFont(u8g2_font_maniac_tr);
    tw = u8g2Fonts.getUTF8Width("IoT Alarm");
    th = (u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent());
    tx = (display.width() - tw)/2;
    ty = 40;
    u8g2Fonts.setCursor(tx, ty+Y_OFFSET);
    u8g2Fonts.println("IoT Alarm");

    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    tw = u8g2Fonts.getUTF8Width("version 1.0");
    th = (u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent());
    tx = (display.width() - tw)/2;
    ty = 60;
    u8g2Fonts.setCursor(tx, ty+Y_OFFSET);
    u8g2Fonts.println("version 1.0");

    tw = u8g2Fonts.getUTF8Width(label);
    th = (u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent());
    tx = (display.width() - tw)/2;
    ty = 105;
    u8g2Fonts.setCursor(tx, ty+Y_OFFSET);
    u8g2Fonts.println(label);
}

/**
 * @brief Wraps a given text to fit within a specified width, breaking it into multiple lines.
 * 
 * This function takes an input string `text` and wraps it to fit within the specified `maxWidth`.
 * It ensures that words are not broken in the middle, and it respects spaces and newline characters 
 * in the input. The resulting wrapped text is stored in the `output` buffer, and the number of lines 
 * generated is returned in the `lines` pointer.
 * 
 * If the text can fit within `maxWidth` as a single line, it returns the entire text as is.
 * If the text is too long, it breaks it into multiple lines based on the given width, inserting 
 * spaces and newlines as needed.
 * 
 * @param text      The input string to be wrapped.
 * @param output    The buffer to store the wrapped text. It should be large enough to hold the result.
 * @param lines     Pointer to an integer where the number of lines will be stored.
 * @param maxWidth  The maximum width available for each line.
 * 
 * @details
 * - The function uses `u8g2Fonts.getUTF8Width` to calculate the width of the text or substring. 
 * - If the text fits within `maxWidth` as a single line, it is returned without modification.
 * - If wrapping is necessary, the function searches for spaces and newline characters to decide where to break the text.
 * - The function ensures that no word is split across lines unless it is too large to fit in the current line, in which case the word will be moved to the next line.
 * 
 * @return None
 * 
 * Example Usage:
 * @code
 * char wrappedText[200];
 * uint8_t numLines;
 * wrapTextToFitWidth("This is a long text that needs to be wrapped to fit the screen.", wrappedText, &numLines, 100);
 * @endcode
 * This will wrap the text to fit within 100 pixels wide, and the wrapped text will be stored in `wrappedText`.
 * The number of lines will be stored in `numLines`.
 */
void wrapTextToFitWidth(const char* text, char* output, uint8_t* lines, uint16_t maxWidth) {
    std::string input(text);
    std::string line;
    std::string result;
    size_t start = 0, end = 0;
    *lines = 0;

    // text fits within maxWidth -> return as a single line
    if (u8g2Fonts.getUTF8Width(input.c_str()) <= maxWidth) {
        result += input;
        *lines = 1;
        strncpy(output, result.c_str(), result.size() + 1);
        return;
    }

    // text wrapping cycle
    while (start < input.size()) {
        end = start;
        size_t nextSpace = input.find(' ', end);
        size_t nextBreak = input.find('\n', end);

        // rest of the text fits within maxWidth -> break wrapping
        if (u8g2Fonts.getUTF8Width(input.substr(start).c_str()) <= maxWidth) {
            result += input.substr(start).c_str();
            *lines += 1;
            break;
        }

        // '\n' character before space -> break the line there
        if (nextBreak != std::string::npos && (nextBreak < nextSpace || nextSpace == std::string::npos)) {
            end = nextBreak + 1;
            line = input.substr(start, end - start - 1);
            result += line;
            *lines += 1;
            start = end;
            continue;
        }

        // current line wrapping cycle
        while (nextSpace != std::string::npos && u8g2Fonts.getUTF8Width(input.substr(start, nextSpace - start).c_str()) <= maxWidth) {
            end = nextSpace + 1;
            nextSpace = input.find(' ', end);
            nextBreak = input.find('\n', end);

            // '\n' character before space -> break the line there
            if (nextBreak != std::string::npos && (nextBreak < nextSpace || nextSpace == std::string::npos)) {
                end = nextBreak + 1;
                break;
            }
        }

        // no spaces found || word exceeds the line -> break at the last valid position
        if (end == start) {
            while (end < input.size() && u8g2Fonts.getUTF8Width(input.substr(start, end - start + 1).c_str()) <= maxWidth) {
                end++;
            }
        }

        // append line to result
        line = input.substr(start, end - start);
        if (!line.empty() && (line.back() == ' ' || line.back() == '\n')) {
            line.pop_back();
        }
        result += line;
        result += '\n';
        *lines += 1;
        start = end;
    }

    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    strncpy(output, result.c_str(), result.size() + 1);
}

void notificationScreenTemplate(const char *label, const char *data) {
    uint16_t x, y, w, h;
    int16_t tx1, ty1;

    u8g2Fonts.setFont(u8g2_font_courB14_tr);
    int16_t labelWidth = u8g2Fonts.getUTF8Width(label);
    int16_t labelHeight = u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent();

    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    char wrappedData[128];
    uint8_t lines;
    wrapTextToFitWidth(data, wrappedData, &lines, display.width() - 24);

    // text wrapping
    char *line = strtok(wrappedData, "\n");
    String lineArray[10];
    int lineWidths[10];
    int maxLineWidth = 0;
    int lineIndex = 0;

    while (line != NULL && lineIndex < 10) {
        lineArray[lineIndex] = String(line);
        lineWidths[lineIndex] = u8g2Fonts.getUTF8Width(line);
        if (lineWidths[lineIndex] > maxLineWidth) {
            maxLineWidth = lineWidths[lineIndex];
        }
        lineIndex++;
        line = strtok(NULL, "\n");
    }

    int16_t lineHeight = u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent();
    int16_t totalDataHeight = lineHeight * lineIndex + 4 * (lineIndex - 1);

    // rect w, h
    w = max((int)labelWidth, maxLineWidth) + 16;
    w = min((int)w, (int)display.width());
    h = labelHeight + 4 + totalDataHeight + 16;
    h = h + (8 - h%8);
    h = min((int)h, (int)display.height() - 6);
    
    // rect coords
    x = (display.width() - w)/2;
    y = (display.height()-6 - h)/2;
    y = y + (8 - y%8);

    // label coords
    tx1 = (display.width() - labelWidth) / 2;
    ty1 = y + labelHeight + 4;

    display.setPartialWindow(x, y, w, h);
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawRect(x, y, w, h, GxEPD_BLACK);

        // display label
        u8g2Fonts.setFont(u8g2_font_courB14_tr);
        u8g2Fonts.setCursor(tx1, ty1);
        u8g2Fonts.print(label);

        // display data
        u8g2Fonts.setFont(u8g2_font_courB10_tr);
        for (int i = 0; i < lineIndex; i++) {
            int16_t tx2 = (display.width() - lineWidths[i]) / 2;
            int16_t ty2 = ty1 + 24 + i * (lineHeight + 4);
            u8g2Fonts.setCursor(tx2, ty2);
            u8g2Fonts.print(lineArray[i]);
        }
    } while (display.nextPage());
}

// ---------------------------------------------------------------------------
// UPDATE FUNCTIONS

void updateSelection(int selection_id) {
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    if (selection_id < 0) {
        u8g2Fonts.setCursor(10, 47+Y_OFFSET);
        u8g2Fonts.print("<");
    } else {
        switch (selection_id) {
            case 0:
                u8g2Fonts.setCursor(20, 47+Y_OFFSET);
                break;
            case 1:
                u8g2Fonts.setCursor(20, 65+Y_OFFSET);
                break;
            case 2:
                u8g2Fonts.setCursor(20, 83+Y_OFFSET);
                break;
            case 3:
                u8g2Fonts.setCursor(20, 101+Y_OFFSET);
                break;
        }
        u8g2Fonts.print(">");
    }
}

void updateDatetime(const char * date, const char * time) {
    u8g2Fonts.setFont(u8g2_font_courB08_tr);
    u8g2Fonts.setCursor(185, 115+Y_OFFSET);
    u8g2Fonts.print(date);
    u8g2Fonts.setCursor(215, 101+Y_OFFSET);
    u8g2Fonts.print(time);
}

void updatePin(String pin, int x, int y) {
    int delimiter = pin.indexOf('#');
    String pin_temp;
    if (delimiter > 0) {
        pin_temp = pin.substring(delimiter+1);
    } else {
        pin_temp = pin;
    }

    String safe_pin;
    int length = pin_temp.length();
    for (int i = 0; i < length; i++) {
        if (pin.charAt(i) == '#') {
            safe_pin+="";
        } else {
            safe_pin+="#";
        }
    }

    u8g2Fonts.setFont(u8g2_font_courB18_tr);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.printf("PIN:%s", safe_pin);
}

void updateAttempts(int attempts, int x, int y) {
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.printf("attempts: %d", attempts);
}

void updateStatusIcons(int wifi, int gsm, int battery) {
    u8g2Fonts.setFont(u8g2_font_siji_t_6x10);
    u8g2Fonts.setCursor(232, 16+Y_OFFSET);
    if (wifi > 0) {
        // wifi is not connected
        u8g2Fonts.print("\ue217");
    }

    if (wifi > -60) {
        u8g2Fonts.print("\ue21a");
    } else if (wifi > -70) {
        u8g2Fonts.print("\ue219");
    } else if (wifi > -85) {
        u8g2Fonts.print("\ue218");
    } else {
        u8g2Fonts.print("\ue217");
    }

    u8g2Fonts.setCursor(217, 16+Y_OFFSET);
    if (battery < 5) {
        u8g2Fonts.print("\ue24c");
    } else if (battery < 0 && battery >= 15) {
        u8g2Fonts.print("\ue24d");
    } else if (battery < 0 && battery >= 25) {
        u8g2Fonts.print("\ue24e");
    } else if (battery < 0 && battery >= 35) {
        u8g2Fonts.print("\ue24f");
    } else if (battery < 0 && battery >= 50) {
        u8g2Fonts.print("\ue250");
    } else if (battery < 0 && battery >= 65) {
        u8g2Fonts.print("\ue251");
    } else if (battery < 0 && battery >= 75) {
        u8g2Fonts.print("\ue252");
    } else if (battery < 0 && battery >= 85) {
        u8g2Fonts.print("\ue253");
    } else if (battery >= 95) {
        u8g2Fonts.print("\ue254");
    }

    u8g2Fonts.setCursor(202, 16+Y_OFFSET);
    if (gsm == 99) {
        // signal not known or detectable
    }
    if (gsm > 19) {
        u8g2Fonts.print("\ue25c");
    } else if (gsm > 14) {
        u8g2Fonts.print("\ue25b");
    } else if (gsm > 9) {
        u8g2Fonts.print("\ue25a");
    } else if (gsm > 1) {
        u8g2Fonts.print("\ue259");
    } else {
        u8g2Fonts.print("\ue258");
    }
}

// ---------------------------------------------------------------------------
// HELPER FUNCTIONS

int getSelectionId(States state, int selection) {
    switch (state) {
        case STATE_INIT:
        switch (selection) {
            case SELECTION_INIT_SETUP:                  return 0;
            case SELECTION_INIT_ALARM:                  return 1;
            case SELECTION_INIT_TEST:                   return 2;
            case SELECTION_INIT_REBOOT:                 return 3;
            default:                                    return -2;
        }

        case STATE_SETUP:
        switch (selection) {
            case SELECTION_SETUP_START_STA:             return 0;
            case SELECTION_SETUP_OPEN_ZB:               return 1;
            case SELECTION_SETUP_CLOSE_ZB:              return 1;
            case SELECTION_SETUP_CLEAR_ZB:              return 1;
            case SELECTION_SETUP_RESET_ZB:              return 1;
            case SELECTION_SETUP_ADD_RFID:              return 2;
            case SELECTION_SETUP_DEL_RFID:              return 2;
            case SELECTION_SETUP_CHECK_RFID:            return 2;
            case SELECTION_SETUP_HARD_RESET:            return 3;
            case SELECTION_SETUP_RETURN:                return -1;
            default:                                    return -2;
        }

        case STATE_ALARM_IDLE:
        switch (selection) {
            case SELECTION_ALARM_IDLE_LOCK:             return 0;
            case SELECTION_ALARM_IDLE_CHANGE_PASSWORD:  return 1;
            case SELECTION_ALARM_IDLE_REBOOT:           return 2;
            case SELECTION_ALARM_IDLE_RETURN:           return -1;
            default:                                    return -2;
        }

        case STATE_TEST_IDLE:
        switch (selection) {
            case SELECTION_TEST_IDLE_LOCK:             return 0;
            case SELECTION_TEST_IDLE_CHANGE_PASSWORD:  return 1;
            case SELECTION_TEST_IDLE_REBOOT:           return 2;
            case SELECTION_TEST_IDLE_RETURN:           return -1;
            default:                                   return -2;
        }
        
        default:
            return -3;
    }
}

void waitReady() {
    while (digitalRead(EPD_BUSY)) {
        vTaskDelay(75 / portTICK_PERIOD_MS);
    }
    return;
}
