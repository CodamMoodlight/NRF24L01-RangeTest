#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include "config.h"

#define CE_PIN 7
#define CSN_PIN 8

RF24 radio(CE_PIN, CSN_PIN);

float payload = 0.0;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

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



    // For debugging info
    // printf_begin();             // needed only once for printing details
    // radio.printDetails();       // (smaller) function that prints raw register values
    // radio.printPrettyDetails(); // (larger) function that prints human readable data
}

void loop()
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

    // to make this example readable in the serial monitor
    delay(1000); // slow transmissions down by 1 second
}