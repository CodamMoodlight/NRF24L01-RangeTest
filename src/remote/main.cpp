#include <SPI.h>
#include "RF24.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LowPower.h>

#define CE_PIN 7
#define CSN_PIN 8

#define BTN_PIN 0
#define LED_PIN 9

// #define PRINT

RF24 radio(CE_PIN, CSN_PIN);
float payload = 0.0;
unsigned long time_old;


void radioSend()
{
    // This device is a TX node
    bool report = radio.write(&payload, sizeof(float)); // transmit & save the report

    if (report)
    {
#ifdef PRINT
        Serial.print(F("Transmission successful! ")); // payload was delivered
        Serial.print(F("Time to transmit = "));
        Serial.print(F(" us. Sent: "));
        Serial.println(payload); // print payload sent
#endif
        payload += 0.01; // increment float payload
    }
    else
    {
#ifdef PRINT
        Serial.println(F("Transmission failed or timed out")); // payload was not delivered
#endif
    }
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
    radio.setPayloadSize(sizeof(payload)); // float datatype occupies 4 bytes

    radio.stopListening();                       // put radio in TX mode
    radio.openWritingPipe(RADIO_ADDR[ADDR_LED]); // always uses pipe 0
}

void interrupt_cb()
{
    if (millis() - time_old > 200)
    {
        radioSend();
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        time_old = millis();
    }
}

void setup()
{
    delay(5000);
    time_old = millis();

    // pinMode(BTN_PIN, INPUT_PULLUP);
    // 32U4 Interrupt pins 0, 1, 2, 3, 7
    // attachInterrupt(digitalPinToInterrupt(BTN_PIN), interrupt_cb, LOW);

    setup_radio();
#ifdef PRINT
    Serial.begin(115200);
#endif

}

void loop()
{
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
    radioSend();
}
