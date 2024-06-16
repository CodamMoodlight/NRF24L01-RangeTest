#include <SPI.h>
#include "RF24.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LowPower.h>
#include <PinChangeInterrupt.h>

#define CE_PIN 3
#define CSN_PIN 4


// #define PRINT
#define LED_COUNT 3

RF24 radio(CE_PIN, CSN_PIN);


typedef void isr_func(void);

void button_1_cb();
void button_2_cb();
void button_3_cb();
void button_4_cb();

const isr_func *ISR_FUNCS[BUTTON_COUNT] = {
    button_1_cb,
    button_2_cb,
    button_3_cb,
    button_4_cb,
};


const int8_t BUTTONS[BUTTON_COUNT] {
    5, 6, 7, 9
};

int counter = 0;

// BUTTON_COUNT is our non-button pressed state
t_button pressed_button = BUTTON_COUNT;

void disable_interrupts()
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        disablePinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTONS[i]));
    }
}

void enable_interrupts()
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        enablePinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTONS[i]));
    }
}


void radioSend(t_button btn)
{
    t_payload payload = set_payload((t_command)(btn), (t_device) (0));
    radio.powerUp();
    for (size_t i = 0; i < LED_COUNT; i++)
    {
        // go from index `ADDR_LED_1` to `i`
        radio.openWritingPipe(RADIO_ADDR[1 + i]);
        bool report = radio.write(&payload, sizeof(t_payload));
        delay(50);
    }
    radio.powerDown();
}

void setup_radio()
{
    // initialize the transceiver on the SPI bus
    if (!radio.begin())
    {
#ifdef PRINT
        Serial.println(F("radio hardware is not responding!!"));
#endif
    }
    // TODO Test powerlevels.
    radio.setPALevel(RF24_PA_HIGH);
    radio.setAddressWidth(3);
    radio.setPayloadSize(sizeof(t_payload));

    radio.stopListening();                       // put radio in TX mode
    radio.openWritingPipe(RADIO_ADDR[ADDR_LED_1]);
}

void wakeup(t_button x)
{
   pressed_button = x; 
}


void button_1_cb() {wakeup(BUTTON_1);}
void button_2_cb() {wakeup(BUTTON_2);}
void button_3_cb() {wakeup(BUTTON_3);}
void button_4_cb() {wakeup(BUTTON_4);}

void setup()
{
#ifdef DEBUG
    Serial.println("Restarting...");
    Serial.begin(115200);
#endif
    delay(5000);

#ifdef DEBUG
    Serial.println("Done");
#endif


    // Setup our Pin Change Interrupts.
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        pinMode(BUTTONS[i], INPUT_PULLUP);
        attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTONS[i]), ISR_FUNCS[i], RISING);
    }


    // Set unused pins to this saves around 100uA.
    // for (int i = 0; i < 20; i++)
    // {
    //     if (i != BUTTONS[0] && i != BUTTONS[1] && i != BUTTONS[2] && i != BUTTONS[3])
    //         pinMode(i, OUTPUT);
    // }
    

    setup_radio();
    radio.powerDown();
}

void loop()
{
    enable_interrupts();

    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

    disable_interrupts();

    if (pressed_button != BUTTON_COUNT)
    {
        radioSend(pressed_button);
        delay(100);
    }

    pressed_button = BUTTON_COUNT;

}