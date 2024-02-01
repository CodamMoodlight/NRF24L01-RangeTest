#include "RF24.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include <SD.h>

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32 
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define PIN_RADIO_D0 6
#define PIN_RADIO_D1 7
#define PIN_RADIO_D2 8
#define PIN_RADIO_D3 9

#define RADIO_CE_PIN 7
#define RADIO_CSN_PIN 8

#define SD_CE_PIN 4

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


Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, OLED_RESET);
RF24 radio(RADIO_CE_PIN, RADIO_CSN_PIN);

t_ui main_ui;
t_button state;

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
    [PIN_LED_RED] = 9,
    [PIN_LED_GREEN] = 10,
    [PIN_LED_BLUE] = 5,
    [PIN_LED_WHITE] = 6,
};

const static ColorRBGW PROFILES[] = {
    {100, 0, 0, 0},
    {0, 100, 0, 0},
    {0, 0, 100, 0},
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
        Serial.print("writing to pin: ");
        Serial.print(PINOUT[i]);
        Serial.print(" with value: ");
        Serial.println(color);
#endif
    }
}

void setup()
{
    Serial.begin(115200);

 
    
    
    delay(2000);

    // Set LED PWM pins to output.
    for (size_t i = 0; i < PIN_COUNT; i++)
    {
        pinMode(PINOUT[i], OUTPUT);
    }
    // Set SPI Chip select pins to output.
    pinMode(RADIO_CSN_PIN, OUTPUT);
    pinMode(SD_CHIP_SELECT_PIN, OUTPUT);
    

    Serial.println(F_CPU);

    Serial.println("starting prog");

    display_init_ui(&main_ui, PIN_NAMES);
    display_setup();

    t_pin data = 3;
    display_update_ui(&main_ui, data);
    display_draw_ui(&main_ui);

    /** Set SCK rate to F_CPU/4. See Sd2Card::setSckRate(). */
    // uint8_t const SPI_HALF_SPEED = 1;

    // RF24 = 10000000hz
    if (!SD.begin(SD_CE_PIN))
    {
        Serial.println("initialization failed. Things to check:");
        Serial.println("1. is a card inserted?");
        Serial.println("2. is your wiring correct?");
        Serial.println("3. did you change the chipSelect pin to match your shield or module?");
        Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");
        while (true);
    }

    Serial.println("SD CARD READY");

    Serial.println("Dumping file content");

    delay(5000);
    File f = SD.open("preset_1.txt");
    if (f)
    {
        while (f.available())
        {
            Serial.print(f.readStringUntil('\0'));
        }
        f.close();
    }
    else
        Serial.println("Error opening file");

    delay(5000);


    if (!radio.begin())
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1);
    }
    delay(100);
    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
    radio.setAddressWidth(3);
    radio.setPayloadSize(sizeof(t_button)); // float datatype occupies 4 bytes

    radio.openReadingPipe(0, RADIO_ADDR[ADDR_LED]); // using pipe 0
    radio.startListening(); // put radio in RX mode

    Serial.println("setup DONE!");

    // For debugging info
}


void loop()
{
    uint8_t pipe;
    if (radio.available(&pipe))
    {
        t_button data;
        uint8_t bytes = radio.getPayloadSize();
        radio.read(&data, bytes);
        Serial.print(F("Received "));
        Serial.print(bytes);
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe);
        Serial.print(F(": "));
        Serial.println(data);

        
        if (data == state)
        {
            set_pwm({0, 0, 0, 0});
            state = (t_button) 10;
        }
        else
        {
            set_pwm(PROFILES[data]);
            state = data;
        }

        // when we receive signal update the `ui` struct with our `pin` variable as index.
        display_update_ui(&main_ui, data);
        // draw the ui with the updated fields.
        display_draw_ui(&main_ui);
    }
}