#include <SPI.h>
#include "RF24.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LowPower.h>

#define CE_PIN 7
#define CSN_PIN 8

#define LED_PIN 9


// #define PRINT

RF24 radio(CE_PIN, CSN_PIN);


// 32U4 Interrupt pins 0, 1, 2, 3, 7
int8_t BUTTONS[BUTTON_COUNT] {
    0, 1, 2, 3
};

void radioSend(t_button btn)
{
    t_button tmp = btn;
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
        while (1)
            ;
    }
    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
    radio.setAddressWidth(3);
    radio.setPayloadSize(sizeof(t_button)); // float datatype occupies 4 bytes

    radio.stopListening();                       // put radio in TX mode
    radio.openWritingPipe(RADIO_ADDR[ADDR_LED]); // always uses pipe 0
}

void button_1_cb() {radioSend(BUTTON_1);}
void button_2_cb() {radioSend(BUTTON_2);}
void button_3_cb() {radioSend(BUTTON_3);}
void button_4_cb() {radioSend(BUTTON_4);}

void setup()
{
    delay(5000);

    pinMode(BUTTONS[0], INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTONS[0]), button_1_cb, LOW);
    pinMode(BUTTONS[1], INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTONS[1]), button_2_cb, LOW);
    pinMode(BUTTONS[2], INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTONS[2]), button_3_cb, LOW);
    pinMode(BUTTONS[3], INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTONS[3]), button_4_cb, LOW);

    setup_radio();
    radio.powerDown();
#ifdef PRINT
    Serial.begin(115200);
#endif

}

void loop()
{
    LowPower.powerDown(SLEEP_FOREVER, ADC_ON, BOD_OFF);
}
