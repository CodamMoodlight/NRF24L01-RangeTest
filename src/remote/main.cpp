#include <SPI.h>
#include "RF24.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LowPower.h>
#include <PinChangeInterrupt.h>

#define CE_PIN 3
#define CSN_PIN 4

// Amout of light fixtures to send the payload to.
#define LED_COUNT 3

#define DEBUG_PIN 10

typedef void isr_func(void);


RF24 radio(CE_PIN, CSN_PIN);

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
t_button g_pressed_button = BUTTON_COUNT;

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
    // construct payload.
    t_payload payload = set_payload(btn, (t_device) 0);
    
    radio.powerUp();
    for (size_t i = 0; i < LED_COUNT; i++)
    {
        // go from index `ADDR_LED_1` to `i`
        // radio.openWritingPipe(RADIO_ADDR[1 + i]);
        radio.openWritingPipe(RADIO_ADDR[1]);
        bool report = radio.write(&payload, sizeof(t_payload)); // transmit & save the report
        if (!report)
        {
            for (size_t i = 0; i < 5; i++)
            {
                digitalWrite(DEBUG_PIN, LOW);
                delay(10);
                digitalWrite(DEBUG_PIN, HIGH);
                delay(10);
            }
        }
        else
        {
            for (size_t i = 0; i < 2; i++)
            {
                digitalWrite(DEBUG_PIN, LOW);
                delay(20);
                digitalWrite(DEBUG_PIN, HIGH);
                delay(20);
            }
        }
#ifdef PRINT
        if (report)
        {
            Serial.print(F("Transmission successful! ")); // payload was delivered
            Serial.print(F("Time to transmit = "));
            Serial.print(F(" us. Sent: "));
            Serial.println(tmp); // print payload sent
        }
        else
        {
            Serial.println(F("Transmission failed or timed out")); // payload was not delivered
        }
        Serial.print("counter: ");
        Serial.println(counter);
        counter++;
#endif
        delay(5);

        break;
    }
    
    radio.powerDown();
}

void setup_radio()
{
#ifdef PRINT
    // initialize the transceiver on the SPI bus
    if (!radio.begin())
    {
        Serial.println(F("radio hardware is not responding!!"));
    }
#endif
    
    radio.setPALevel(RF24_PA_HIGH);
    radio.setAddressWidth(3);
    radio.setPayloadSize(sizeof(t_payload));

    // put radio in TX mode
    radio.stopListening();
    radio.openWritingPipe(RADIO_ADDR[ADDR_LED_1]);
}

void wakeup(t_button x)
{
   g_pressed_button = x; 
}


void button_1_cb() {wakeup(BUTTON_1);}
void button_2_cb() {wakeup(BUTTON_2);}
void button_3_cb() {wakeup(BUTTON_3);}
void button_4_cb() {wakeup(BUTTON_4);}

void setup()
{
    delay(5000);
#ifdef DEBUG
    Serial.begin(115200);
    // Serial.println("Restarting...");
    // Serial.println("Done");

    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, HIGH);
#endif

    // Setup our Pin Change Interrupts.
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        pinMode(BUTTONS[i], INPUT_PULLUP);
        attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTONS[i]), ISR_FUNCS[i], RISING);
    }


    // TODO Checkout how much this matters
    // Set unused pins to this saves around 100uA.
    // for (int i = 0; i < 20; i++)
    // {
    //     if (i != BUTTONS[0] && i != BUTTONS[1] && i != BUTTONS[2] && i != BUTTONS[3])
    //         pinMode(i, OUTPUT);
    // }
    

    setup_radio();
    radio.powerDown();

#ifdef DEBUG
    digitalWrite(DEBUG_PIN, LOW);
    delay(100);
    digitalWrite(DEBUG_PIN, HIGH);
    delay(100);
    digitalWrite(DEBUG_PIN, LOW);
#endif
}

void loop()
{
    enable_interrupts();

    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    // When an interrupt gets called the device will wakeup and finish the remaning of the `loop` function.
    // Until it reaches the `powerDown` call again.

    disable_interrupts();

    if (g_pressed_button != BUTTON_COUNT)
    {
        radioSend(g_pressed_button);
#ifdef DEBUG
        digitalWrite(DEBUG_PIN, HIGH);
        delay(100);
        digitalWrite(DEBUG_PIN, LOW);
        delay(100);
#endif
        delay(100);
    }

    g_pressed_button = BUTTON_COUNT;

}