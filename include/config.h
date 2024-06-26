#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef enum e_addreses
{
    ADDR_REMOTE,
    ADDR_LED,
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

static const uint8_t RADIO_ADDR[ADDR_COUNT][6] = {
    "REM",
    "LED"
};


// Use a `t_payload` type for sending our data.
typedef uint8_t t_payload;

#endif