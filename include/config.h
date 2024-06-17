#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>


/************************/
// Light fixture macros //
/************************/
// Change this value depending on what light you're building the firmware for.
// #define CURRENT_READING_PIPE ADDR_LED_1
// #define CURRENT_READING_PIPE ADDR_LED_2
#define CURRENT_READING_PIPE ADDR_LED_3
// #define CURRENT_READING_PIPE ADDR_TEST

// #define HAS_LCD
#define DEBUG
/************************/






typedef enum e_addreses
{
    ADDR_REMOTE,
    ADDR_LED_1,
    ADDR_LED_2,
    ADDR_LED_3,
    ADDR_TEST,
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
static const uint8_t RADIO_ADDR[ADDR_COUNT + 1][6] = {
    "REM",
    "LD1",
    "LD2",
    "LD3",
    "TST",
};


// Use a `t_payload` type for sending our data.
typedef uint8_t t_payload;
// the payload looks like: 0b0000 0000
// where the first nibble contains the `t_command` bits, and the last the `t_device` where the light fixture(s) will clear its bit if it has received the payload forwarding it to the one that hasn't received it.
// So when sending this payload from the remote, it should all be set.

typedef enum e_command
{
	COMMAND_BUTTON_1 = 0,
	COMMAND_BUTTON_2 = 1,
	COMMAND_BUTTON_3 = 2,
	COMMAND_BUTTON_4 = 3,
	COMMAND_BUTTON_1_OFF = 4,
    COMMAND_COUNT,
} t_command;

typedef enum e_device {
	DEVICE_LD1 = (1U << 0),
	DEVICE_LD2 = (1U << 1),
	DEVICE_LD3 = (1U << 2),
	DEVICE_TEST = (1U << 3),
} t_device;




t_payload set_payload(t_command cmd, t_device devices)
{
	return (cmd << 4) | devices;
}

t_command get_command(t_payload p)
{
	return (t_command) (p >> 4);
}

t_device get_devices(t_payload p)
{
	return (t_device) (p & 15);
}


#endif