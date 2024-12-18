/**
 * @file libDisplay.h
 * @brief Contains functions and definitions for controlling display.
 * 
 * Contains functions and definitions for controlling display.
 */

#ifndef LIBDISPLAY_H_DEFINITION
#define LIBDISPLAY_H_DEFINITION

#include <Arduino.h>
#include <WiFi.h>

#include "utils.h"
#include "mainAppDefinitions.h"

#define LCD_COLS 20
#define LCD_ROWS 4
#define LCD_ADDR 0x27

enum notificationScreenId {
    NOTIFICATION_NONE,
    NOTIFICATION_AUTH_CHECK_SUCCESS,
    NOTIFICATION_AUTH_CHECK_ERROR,
    NOTIFICATION_AUTH_SET_SUCCESS,
    NOTIFICATION_AUTH_SET_ERROR,
    NOTIFICATION_RFID_CHECK_SUCCESS,
    NOTIFICATION_RFID_CHECK_ERROR,
    NOTIFICATION_RFID_ADD_SUCCESS,
    NOTIFICATION_RFID_ADD_ERROR,
    NOTIFICATION_RFID_DEL_SUCCESS,
    NOTIFICATION_RFID_DEL_ERROR,
    NOTIFICATION_ZIGBEE_NET_OPEN,
    NOTIFICATION_ZIGBEE_NET_CLOSE,
    NOTIFICATION_ZIGBEE_NET_CLEAR,
    NOTIFICATION_ZIGBEE_NET_RESET,
    NOTIFICATION_ZIGBEE_ATTR_REPORT,
    NOTIFICATION_ZIGBEE_DEV_ANNCE,
    NOTIFICATION_ZIGBEE_DEV_LEAVE,
    NOTIFICATION_ZIGBEE_DEV_COUNT,
    NOTIFICATION_MQTT_CONNECTED,
    NOTIFICATION_MQTT_DISCONECTED,
    NOTIFICATION_WIFI_CONNEDTED,
    NOTIFICATION_WIFI_DISCONECTED,
    NOTIFICATION_MAX,
};

enum updateScreenParam {
    UPDATE_NONE,
    UPDATE_SELECTION,
    UPDATE_DATETIME,
    UPDATE_STATUS,
    UPDATE_PIN,
    UPDATE_ATTEMPTS,
    UPDATE_ALARM_STATUS,
    UPDATE_EVENTS,
    UPDATE_COUNTDOWN,
    UPDATE_MAX,
};

typedef struct {
    notificationScreenId id;
    int param;
    int duration;
} notification_t;

/**
 * @brief Initializes the e-ink display and sets up the communication interface.
 * 
 * This function initializes the e-ink display, configures the SPI communication, 
 * and sets various display settings such as rotation, text color, and font. It also 
 * shows an initial screen with the provided template. The e-ink display is initialized 
 * for use with the `GxEPD` library, with the SPI communication set up using the provided 
 * clock, MOSI, MISO pins, and other configuration parameters.
 * 
 * @details
 * - The SPI communication interface is initialized using the specified pins.
 * - The display is configured for the required settings such as rotation, text color, 
 *   and window size.
 * - The function also prints debug information about the display's number of pages 
 *   and page height to the serial monitor.
 * - An initial screen is displayed with a template, showing a border and a label.
 * 
 * This function assumes the e-ink display has been wired correctly to the microcontroller 
 * and that the appropriate libraries for SPI and `GxEPD` are included in the project.
 * 
 * @code
 * initEink();
 * @endcode
 * 
 * @return void
 */
void initEink();

/**
 * @brief Restarts the e-ink display by clearing it and showing a reboot message.
 * 
 * This function clears the e-ink display, draws a border around the screen, and 
 * displays a message indicating that the system is rebooting. It uses a partial 
 * window update to refresh the display content, ensuring a clean restart screen is shown.
 * 
 * @details
 * - The function sets a partial window to refresh the entire display area.
 * - The display is filled with a white background, and a border is drawn around the 
 *   screen with a black outline.
 * - The message "Rebooting..." is displayed in the center using the provided 
 *   screen template function.
 * - The function uses `firstPage()` and `nextPage()` to handle the display refresh, 
 *   which is typical for e-ink displays to ensure the content is properly rendered.
 * 
 * This function is useful when performing a soft reboot of the device, as it provides 
 * a visual cue on the e-ink display while the device is rebooting.
 * 
 * @code
 * displayRestart();
 * @endcode
 * 
 * @return void
 */
void displayRestart();

/**
 * @brief Updates and refreshes the display based on the current state of the system.
 * 
 * This function checks the `refresh_display` flags to determine if different parts of the display need to be updated. 
 * It handles refreshing the main screen, the selection window, status icons, datetime, and PIN input areas based on the system state. 
 * The function uses the `GxEPD` library to update portions of the screen efficiently, ensuring the display is updated without redrawing 
 * the entire screen. Each display update is performed conditionally based on the active state and flags, optimizing the refresh rate.
 * 
 * @param None
 * 
 * @return None
 * 
 * @details
 * The function first checks if a complete screen refresh is required by checking `g_vars_ptr->refresh_display.refresh`. 
 * If the flag is set, it starts a full refresh of the screen, redrawing the entire screen contents including borders, 
 * menus, and status information according to the current state of the system. 
 * 
 * If a partial refresh is required (e.g., only the selection window, status icons, datetime, or PIN), the function uses 
 * `setPartialWindow` to only refresh the necessary areas of the screen, ensuring efficient screen updates.
 * 
 * Several states are handled specifically, such as:
 * - `STATE_SETUP`: Updates the Zigbee and RFID configuration menu.
 * - `STATE_ALARM_OK` and `STATE_TEST_OK`: Display alarm status and event details.
 * - `STATE_SETUP_PIN1`, `STATE_SETUP_PIN2`, `STATE_SETUP_PIN3`: Handle PIN input for system setup.
 * 
 * If none of the defined states match, a warning is logged for unrecognized states.
 * 
 * Example code:
 * ```cpp
 * display.setPartialWindow(10, 32+Y_OFFSET_8th_high, 20, 80+Y_OFFSET_8th_low);
 * display.firstPage();
 * do {
 *     display.fillScreen(GxEPD_WHITE);
 *     updateSelection(getSelectionId(g_vars_ptr->state, g_vars_ptr->selection));
 * } while (display.nextPage());
 * ```
 */
void displayLoad();

/**
 * @brief Handles different notification scenarios and displays appropriate messages on the screen.
 * 
 * This function processes different types of notifications, such as successful authentication, 
 * RFID events, Zigbee network actions, and connectivity changes (WiFi, MQTT). It calls a 
 * function to display the corresponding message based on the notification type and the 
 * associated parameters.
 * 
 * @param notification The type of notification to display. This is an enumeration of possible 
 *                     notification types (e.g., `NOTIFICATION_AUTH_CHECK_SUCCESS`, 
 *                     `NOTIFICATION_RFID_CHECK_SUCCESS`).
 * @param param Additional parameter for some notification types, such as the time for Zigbee 
 *              network joining or the number of devices connected to the Zigbee network.
 * 
 * @return void
 * 
 * @details
 * This function processes various notifications and displays messages based on the notification 
 * type. Each notification corresponds to an event that has occurred in the system, such as a 
 * successful or failed authentication attempt, the status of a Zigbee network, or changes in 
 * network connectivity (WiFi, MQTT). Some notifications also use an additional parameter (`param`), 
 * which is typically used for time-based events (e.g., Zigbee network joining time) or counting devices 
 * connected to the Zigbee network.
 * 
 * The function uses a helper function `notificationScreenTemplate` to display the notification message 
 * on the screen. After handling the notification, the `waitReady` function is called to ensure that 
 * the system is ready for the next operation.
 * 
 * @code
 * // Example usage:
 * displayNotificationHandler(NOTIFICATION_AUTH_CHECK_SUCCESS, 0);
 * // This will display "Correct PIN" and "Access permitted!" on the screen.
 * @endcode
 */
void displayNotificationHandler(notificationScreenId notification, int param = 0);

/**
 * @brief Sends a notification to the notification queue.
 * 
 * This function creates a notification structure, initializes it with the provided 
 * parameters, and sends it to a queue for further processing. If memory allocation 
 * or queue operation fails, an appropriate log message is generated. Otherwise, 
 * it logs the successful enqueueing of the notification.
 * 
 * @param id The notification screen ID (of type `notificationScreenId`) which 
 *           identifies the type or category of the notification to be displayed.
 * @param param An integer parameter associated with the notification. This can be 
 *              used to pass additional data relevant to the notification (e.g., 
 *              notification parameters).
 * @param duration The duration (in milliseconds) for which the notification will be 
 *                 displayed or active.
 * 
 * @return None
 * 
 * @details
 * This function allocates memory for a `notification_t` structure and assigns the 
 * provided `id`, `param`, and `duration` values to the structure fields. The notification 
 * is then enqueued to a notification queue (`queueNotification`) with a timeout of 10 milliseconds.
 * 
 * If memory allocation fails, an error log is generated and the function exits. Similarly, 
 * if the notification cannot be added to the queue, a warning log is generated. Upon success, 
 * an info log confirms the notification has been enqueued.
 * 
 * Example usage:
 * @code
 * displayNotification(NOTIF_TYPE_ERROR, 1, 5000);
 * @endcode
 * This would display an error notification with a 5-second duration and a parameter of 1.
 */
void displayNotification(notificationScreenId notification, int param = 0, int duration = 0);

/**
 * @brief Displays a notification screen with a label and wrapped data.
 * 
 * This function generates a notification screen with a specified label and data. 
 * The data is wrapped to fit within the available screen width, and both the label and 
 * data are centered in a rectangular area on the screen. The rectangle's size adjusts 
 * to fit the content while maintaining padding around the edges. The notification is 
 * displayed on the screen in a clean, formatted manner with appropriate font styles.
 * 
 * @param label The text label to be displayed at the top of the notification screen.
 *              It will be centered above the data section.
 * @param data The content (message) to be displayed below the label. The text will 
 *             be wrapped if it exceeds the available width.
 * 
 * @return None
 * 
 * @details
 * The function first calculates the dimensions of the label and the wrapped data. It
 * then adjusts the size and position of the rectangle based on the content. The 
 * label is displayed in a larger font, while the data is displayed in a smaller font, 
 * with line wrapping applied to ensure the data fits within the screen width. The 
 * notification screen is drawn in a partial window, and the content is updated page by page 
 * for optimized drawing performance on an e-paper display.
 * 
 * @code
 * const char *label = "Notification";
 * const char *data = "This is a message that might be too long to fit on a single line.";
 * notificationScreenTemplate(label, data);
 * @endcode
 */void notificationScreenTemplate(const char * label, const char * data);

#endif