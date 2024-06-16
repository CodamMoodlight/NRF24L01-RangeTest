#include "RF24.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"


#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32 
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define PIN_RADIO_D0 6
#define PIN_RADIO_D1 7
#define PIN_RADIO_D2 8
#define PIN_RADIO_D3 9

#define RADIO_CE_PIN 19
#define RADIO_CSN_PIN 18

#define SD_CE_PIN 8

#define ARRAY_SIZE(x) ((sizeof x) / (sizeof *x))

static const int RADIO_PINS[] = {
    PIN_RADIO_D0,
    PIN_RADIO_D1,
    PIN_RADIO_D2,
    PIN_RADIO_D3
};

static const String PIN_NAMES[] = {
    "D0",
    "D1",
    "D2",
    "D3"
};

typedef struct s_row {
    String name;
    uint32_t data;
} t_row;

typedef struct s_ui {
    t_row rows[ARRAY_SIZE(RADIO_PINS)];
} t_ui;

typedef uint32_t t_pin;


#ifdef HAS_LCD
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, OLED_RESET);
#endif
RF24 radio(RADIO_CE_PIN, RADIO_CSN_PIN);

t_ui main_ui;

#ifdef HAS_LCD
void display_setup()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.display();
}

void display_init_ui(t_ui *ui, const String col_names[])
{
    for (size_t i = 0; i < ARRAY_SIZE(ui->rows); i++)
    {
        ui->rows[i].name = col_names[i];
        ui->rows[i].data = 0;
        Serial.print("ui.rows.name: ");
        Serial.println(col_names[i]);
    }
}

void  display_update_ui(t_ui *ui, t_pin pin)
{
    ui->rows[pin].data++;
}

void display_draw_ui(t_ui *ui)
{

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE); // Draw white text
    for (size_t i = 0; i < ARRAY_SIZE(RADIO_PINS); i++)
    {
        int16_t x = 0;
        int16_t y = 0;

        // generate grid layout.
        if (i % 2)
            y = (DISPLAY_HEIGHT / 2);
        if (i / 2)
            x = (DISPLAY_WIDTH / 2);
        display.setCursor(x, y);

        display.print(ui->rows[i].name);
        display.print(":");
        display.print(ui->rows[i].data);
    }
    // update display
    display.display();
}
#endif

enum PINOUT {
    PIN_LED_RED,
    PIN_LED_GREEN,
    PIN_LED_BLUE,
    PIN_LED_WHITE,
    PIN_COUNT,
};

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
} ColorRBGW;

const static int PINOUT[PIN_COUNT] = {
    [PIN_LED_RED] = 5,
    [PIN_LED_GREEN] = 10,
    [PIN_LED_BLUE] = 9,
    [PIN_LED_WHITE] = 6,
};


// const static ColorRBGW PROFILES[] = {
//     {0, 0, 0, 50}, //power: white
//     {0, 100, 0, 0}, //live: green
//     {100, 0, 0, 0}, //recording: red
//     {0, 0, 100, 0}, //quiet: blue
// };

// Values can be from 0-255
const static ColorRBGW PROFILES[] = {
    {100, 0, 0, 0},
    {10, 100, 0, 0},
    {0, 25, 100, 0},
    {0, 0, 0, 100},
};

void set_pwm(ColorRBGW c)
{
    uint8_t *base = &c.r;
    for (size_t i = 0; i < PIN_COUNT; i++)
    {
        uint8_t color = (uint8_t) base[i];
        analogWrite(PINOUT[i], color);

#ifdef DEBUG
        Serial.print("writing to analog pin: ");
        Serial.print(PINOUT[i]);
        Serial.print(" with value: ");
        Serial.println(color);
#endif
    }
}

void setup()
{

#ifdef DEBUG
    delay(2000);
    Serial.begin(115200);
#endif

    int32_t size = sizeof(t_payload);
    Serial.print("sizeof(t_payload): ");
    Serial.println(size);

    // Set LED PWM pins to output.
    for (size_t i = 0; i < PIN_COUNT; i++)
    {
        pinMode(PINOUT[i], OUTPUT);
    }
    // Set SPI Chip select pins to output.
    pinMode(RADIO_CSN_PIN, OUTPUT);
    
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


    delay(100);

    if (!radio.begin())
    {
#ifdef DEBUG
        Serial.println(F("radio hardware is not responding!!"));
#endif
        while (1)
        {
            for (size_t i = 0; i < PIN_COUNT; i++)
            {
                analogWrite(PINOUT[i], 0);
            }
            delay(500);
            for (size_t i = 0; i < PIN_COUNT; i++)
            {
                analogWrite(PINOUT[i], 255);
            }
            delay(500);
        }

    }
    delay(100);
    radio.setPALevel(RF24_PA_HIGH); // RF24_PA_MAX is default.
    radio.setAddressWidth(3);
    radio.setPayloadSize(sizeof(t_payload));

    radio.openReadingPipe(0, RADIO_ADDR[CURRENT_READING_PIPE]); // using pipe 0
    radio.startListening(); // put radio in RX mode

#ifdef DEBUG
    Serial.println("setup DONE!");
#endif

}



void loop()
{
    uint8_t pipe;
    if (radio.available(&pipe))
    {
        t_payload payload;
        uint8_t bytes = radio.getPayloadSize();
        radio.read(&payload, bytes);
#ifdef DEBUG
        Serial.print(F("Received "));
        Serial.print(bytes);
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe);
        Serial.print(F(": "));
        Serial.println(payload);
#endif

        uint8_t num = 0;
        t_button button = get_button(payload);
        // while (((button >> num) & 0x1) == 0)
        // {
        //     num++;
        //     if (num > 8)
        //         break;
        //         // TODO Handle error
        // }

#ifdef DEBUG


        Serial.print("Number from bin: ");
        Serial.print(button);
        Serial.print(" is numeric: ");
        Serial.println(num);
        


        t_device devices = get_device(payload);
        Serial.print("devices: ");
        Serial.println(devices);
#endif


        set_pwm(PROFILES[num]);

#ifdef HAS_LCD
        // when we receive signal update the `ui` struct with our `pin` variable as index.
        display_update_ui(&main_ui, data);
        // draw the ui with the updated fields.
        display_draw_ui(&main_ui);
#endif
    }
}