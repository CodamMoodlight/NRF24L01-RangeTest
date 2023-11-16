#include <SPI.h>
#include "RF24.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LowPower.h>

#define CE_PIN 7
#define CSN_PIN 8

#define LED_PIN 9


#define PRINT

RF24 radio(CE_PIN, CSN_PIN);


int8_t BUTTONS[BUTTON_COUNT] {
    0, 1, 2, 3
};

#define BUTTON1 2
#define BUTTON2 3

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
        {
            digitalWrite(LED_BUILTIN, 1);
            delay(1000);
        }
    }
    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
    radio.setAddressWidth(3);
    radio.setPayloadSize(sizeof(t_button)); // float datatype occupies 4 bytes

    radio.stopListening();                       // put radio in TX mode
    radio.openWritingPipe(RADIO_ADDR[ADDR_LED]); // always uses pipe 0
}

void button_1_cb() {radioSend(BUTTON_1);}
void button_2_cb() {radioSend(BUTTON_2);}

void setup()
{
    delay(5000);
#ifdef PRINT
    Serial.begin(9600);
    Serial.println("Test123");
#endif


    pinMode(LED_BUILTIN, OUTPUT);
    // pinMode(BUTTON1, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(BUTTON1), button_1_cb, LOW);
    // pinMode(BUTTON2, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(BUTTON2), button_2_cb, LOW);

    
    // TODO Set unused pins to OUTPUT.
    


    setup_radio();
    radio.powerDown();

}

void loop()
{
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
    radioSend(BUTTON_2);
}