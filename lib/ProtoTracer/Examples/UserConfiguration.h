/**
 * @file UserConfiguration.h
 * @brief User-configurable settings for the project.
 *
 * This file defines compile-time macros and settings that enable or disable
 * various features and behaviors of the project. Adjust the definitions below
 * to customize the functionality as needed.
 *
 * @date 22/12/2024
 * @author Coela Can't
 */

#pragma once

/**
 * @def HUB75_RBG
 * @brief Define this macro to use a modified Matrix Hardware file for panels with RBG color order.
 */
//#define HUB75_RBG
#if DOXYGEN
#define HUB75_RBG
#endif

/**
 * @def PRINTINFO
 * @brief Define this macro to enable printing live stats, such as FPS and other information.
 */
#define PRINTINFO
#if DOXYGEN
#define PRINTINFO
#endif

/**
 * @def DEBUG
 * @brief Define this macro to enable debug information output.
 */
//#define DEBUG
#if DOXYGEN
#define DEBUG
#endif

/**
 * @def NEOTRELLISMENU
 * @brief Define this macro to enable the NeoTrellis controller for menu navigation.
 *
 * If this macro is undefined, the button controller is used instead.
 */
//#define NEOTRELLISMENU
#if DOXYGEN
#define NEOTRELLISMENU
#endif

/**
 * @def MORSEBUTTON
 * @brief Define this macro to enable Morse code input using a button.
 *
 * Note: This feature cannot be used simultaneously with NEOTRELLISMENU.
 */
//#define MORSEBUTTON
#if DOXYGEN
#define MORSEBUTTON
#endif

/**
 * @def WIRELESSBUTTON
 * @brief Define this macro when an RF keyfob receiver is plugged into the button port instead of a button.
 *
 * The receiver output is active-high (a button pulls the pin low), so the polarity is flipped, brief radio
 * dropouts are filtered, and a permanently high line (receiver unplugged) is ignored. Menu behaviour is
 * unchanged: short press steps the value, hold moves to the next menu. The receiver must be powered from
 * 3.3V. Not used with NEOTRELLISMENU or MORSEBUTTON.
 */
#define WIRELESSBUTTON
#if DOXYGEN
#define WIRELESSBUTTON
#endif
