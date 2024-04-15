#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>


/************************/
// Light fixture macros //
/************************/
// Change this value depending on what light you're building the firmware for.
// #define CURRENT_READING_PIPE ADDR_LED_1
#define CURRENT_READING_PIPE ADDR_LED_2
// #define CURRENT_READING_PIPE ADDR_LED_3

// #define HAS_LCD
// #define DEBUG
/************************/





typedef enum e_addreses
{
    ADDR_REMOTE,
    ADDR_LED_1,
    ADDR_LED_2,
    ADDR_LED_3,
    ADDR_COUNT,
} t_addreses;

typedef enum e_button
{
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
    BUTTON_COUNT,
} t_button;

// Due to garbage values and because of `setAddressWidth(3);` address "names" have to be 3 chars long.
static const uint8_t RADIO_ADDR[ADDR_COUNT][6] = {
    "REM",
    "LD1",
    "LD2",
    "LD3",
};


// Use a `t_payload` type for sending our data.
typedef uint8_t t_payload;

#endif