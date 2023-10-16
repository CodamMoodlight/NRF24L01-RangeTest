#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define CE_PIN 7
#define CSN_PIN 8

#define BTN_PIN 5


RF24 radio(CE_PIN, CSN_PIN);
float payload = 0.0;

void setup()
{
    pinMode(BTN_PIN, INPUT_PULLUP);
    Serial.begin(115200);

    // initialize the transceiver on the SPI bus
    if (!radio.begin())
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1)
            ;
    }
    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
    radio.setAddressWidth(3);
    radio.setPayloadSize(sizeof(payload)); // float datatype occupies 4 bytes

    radio.stopListening();                       // put radio in TX mode
    radio.openWritingPipe(RADIO_ADDR[ADDR_LED]); // always uses pipe 0
}

void radioSend()
{
    // This device is a TX node
    unsigned long start_timer = micros();               // start the timer
    bool report = radio.write(&payload, sizeof(float)); // transmit & save the report
    unsigned long end_timer = micros();                 // end the timer

    if (report)
    {
        Serial.print(F("Transmission successful! ")); // payload was delivered
        Serial.print(F("Time to transmit = "));
        Serial.print(end_timer - start_timer); // print the timer result
        Serial.print(F(" us. Sent: "));
        Serial.println(payload); // print payload sent
        payload += 0.01;         // increment float payload
    }
    else
    {
        Serial.println(F("Transmission failed or timed out")); // payload was not delivered
    }
}
void loop()
{
    if (!digitalRead(BTN_PIN))
    {
        radioSend();
        delay(200);
    }
}