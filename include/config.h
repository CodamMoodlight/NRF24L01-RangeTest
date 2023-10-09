#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef enum e_addreses
{
    ADDR_REMOTE,
    ADDR_LED,
    ADDR_COUNT,
} t_addreses;

static const uint8_t RADIO_ADDR[ADDR_COUNT][6] = {
    "REM",
    "LED"
};


// Use a `t_payload` type for sending our data.
typedef uint8_t t_payload;

#endif