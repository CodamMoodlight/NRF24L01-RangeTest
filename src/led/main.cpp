#include <SPI.h>
#include "RF24.h"
#include "config.h"

#define CE_PIN 7
#define CSN_PIN 8

RF24 radio(CE_PIN, CSN_PIN);

float payload = 0.0;

void setup()
{
    Serial.begin(115200);
    

    while (!Serial);

    if (!radio.begin())
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1);
    }

    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
    radio.setAddressWidth(3);
    radio.setPayloadSize(sizeof(payload)); // float datatype occupies 4 bytes

    radio.openReadingPipe(0, RADIO_ADDR[ADDR_LED]); // using pipe 0
    radio.startListening(); // put radio in RX mode

    // For debugging info
    // printf_begin();             // needed only once for printing details
    // radio.printDetails();       // (smaller) function that prints raw register values
    // radio.printPrettyDetails(); // (larger) function that prints human readable data
}

void loop()
{
    uint8_t pipe;
    if (radio.available(&pipe))
    {                                           // is there a payload? get the pipe number that recieved it
        uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
        radio.read(&payload, bytes);            // fetch payload from FIFO
        Serial.print(F("Received "));
        Serial.print(bytes); // print the size of the payload
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe); // print the pipe number
        Serial.print(F(": "));
        Serial.println(payload); // print the payload's value
    }
}