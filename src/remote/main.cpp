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
    5, 6, 7, 8
};

int counter = 0;

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
    t_button tmp = btn;
    disable_interrupts();
    radio.powerUp();


    bool report = radio.write(&tmp, sizeof(t_button)); // transmit & save the report

    if (report)
    {
#ifdef PRINT
        Serial.print(F("Transmission successful! ")); // payload was delivered
        Serial.print(F("Time to transmit = "));
        Serial.print(F(" us. Sent: "));
        Serial.println(tmp); // print payload sent
#endif
    }
    else
    {
#ifdef PRINT
        Serial.println(F("Transmission failed or timed out")); // payload was not delivered
#endif
        Serial.println(F("Transmission failed or timed out")); // payload was not delivered
    }
#ifdef PRINT
    Serial.print("counter: ");
    Serial.println(counter);
    counter++;
#endif
    radio.powerDown();
    enable_interrupts();
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
    radio.setPALevel(RF24_PA_LOW);
    radio.setAddressWidth(3);
    radio.setPayloadSize(sizeof(t_button));

    radio.stopListening();                       // put radio in TX mode
    radio.openWritingPipe(RADIO_ADDR[ADDR_LED]); // always uses pipe 0
}

// Gnarly callbacks, but for now this is the easiest way to pass which button has been pressed.
void button_1_cb() {radioSend(BUTTON_1);}
void button_2_cb() {radioSend(BUTTON_2);}
void button_3_cb() {radioSend(BUTTON_3);}
void button_4_cb() {radioSend(BUTTON_4);}

void setup()
{
    Serial.begin(115200);
    Serial.println("Restarting...");
    delay(5000);
    Serial.println("Done");


    pinMode(LED_BUILTIN, OUTPUT);

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
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}