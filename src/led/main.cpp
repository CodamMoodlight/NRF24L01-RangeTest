#include "RF24.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include <SD.h>

#define LED_OFF (t_button) 5

#define RADIO_CE_PIN 19
#define RADIO_CSN_PIN 18

#define SD_CE_PIN 8

RF24 radio(RADIO_CE_PIN, RADIO_CSN_PIN);

t_button state;

typedef struct {
    uint8_t power;
    uint8_t live;
    uint8_t recording;
    uint8_t quiet;
} ButtonColors;

const static ButtonColors PROFILES[] = {
    {0, 0, 0, 50}, //power: white
    {0, 100, 0, 0}, //live: green
    {100, 0, 0, 0}, //recording: red
    {0, 0, 100, 0}, //quiet: blue
};

enum PINOUT {
    PIN_LED_POWER,
    PIN_LED_LIVE,
    PIN_LED_REC,
    PIN_LED_QUIET,
    PIN_COUNT,
};

const static int PINOUT[PIN_COUNT] = {
    [PIN_LED_POWER] = 5,
    [PIN_LED_LIVE] = 10,
    [PIN_LED_REC] = 9,
    [PIN_LED_QUIET] = 6,
};

void set_pwm(ButtonColors c)
{
    uint8_t *base = &c.power;
    for (size_t i = 0; i < PIN_COUNT; i++)
    {
        uint8_t color = (uint8_t) base[i];
        analogWrite(PINOUT[i], color);

#ifdef DEBUG
        Serial.print("writing to pin: ");
        Serial.print(PINOUT[i]);
        Serial.print(" with value: ");
        Serial.println(color);
#endif
    }
}

void setup()
{

#ifdef DEBUG
    Serial.begin(115200);
    delay(2000);
#endif

    // Set LED PWM pins to output.
    for (size_t i = 0; i < PIN_COUNT; i++)
    {
        pinMode(PINOUT[i], OUTPUT);
    }
    // Set SPI Chip select pins to output.
    pinMode(RADIO_CSN_PIN, OUTPUT);
    pinMode(SD_CHIP_SELECT_PIN, OUTPUT);
    
#ifdef DEBUG
    Serial.println(F_CPU);
    Serial.println("starting prog");
#endif
#ifdef HAS_LCD
    t_pin data = 3;
    display_init_ui(&main_ui, PIN_NAMES);
    display_setup();

    display_update_ui(&main_ui, data);
    display_draw_ui(&main_ui);
#endif


    /** Set SCK rate to F_CPU/4. See Sd2Card::setSckRate(). */
    // uint8_t const SPI_HALF_SPEED = 1;

    // RF24 = 10000000hz
    if (!SD.begin(SD_CE_PIN))
    {
        // TODO Flash led
#ifdef DEBUG
        Serial.println("initialization failed. Things to check:");
        Serial.println("1. is a card inserted?");
        Serial.println("2. is your wiring correct?");
        Serial.println("3. did you change the chipSelect pin to match your shield or module?");
        Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");
#endif
        while (true);
    }
#ifdef DEBUG
    Serial.println("SD CARD READY");
    Serial.println("Dumping file content");
#endif
    delay(100);
    File f = SD.open("preset_1.txt");
    if (f)
    {
        while (f.available())
        {
            Serial.print(f.readStringUntil('\0'));
        }
        f.close();
    }
    else
        Serial.println("Error opening file");

    delay(100);


    if (!radio.begin())
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1);
    }
    delay(100);
    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
    radio.setAddressWidth(3);
    radio.setPayloadSize(sizeof(t_button)); // float datatype occupies 4 bytes

    radio.openReadingPipe(0, RADIO_ADDR[ADDR_LED]); // using pipe 0
    radio.startListening(); // put radio in RX mode

    Serial.println("setup DONE!");

    // For debugging info
}

void loop()
{
    uint8_t pipe;
    if (radio.available(&pipe))
    {
        t_button instruction;
        uint8_t bytes = radio.getPayloadSize();
        radio.read(&instruction, bytes);
   
        if (instruction == 0) //if power button is pressed
		{
			if (state != LED_OFF) //if light is on, turn it off
			{
         		set_pwm({0, 0, 0, 0});
				state = LED_OFF;
			}
			else if (state == LED_OFF) //if light is off, turn it on
			{
				set_pwm(PROFILES[instruction]);
				state = instruction;
			}
		}
        else if (state != LED_OFF && instruction != state) //else if light is on and button pressed is not current state, change state
        {
   	    	set_pwm(PROFILES[instruction]);
   	    	state = instruction;
		}
	}
}