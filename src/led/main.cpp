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

#define CE_PIN 7
#define CSN_PIN 8

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
RF24 radio(CE_PIN, CSN_PIN);

t_ui main_ui;
float payload = 0.0;



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

void setup()
{
    Serial.begin(115200);

    display_init_ui(&main_ui, PIN_NAMES);
    display_setup();
    

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

    Serial.println("setup DONE!");

    // For debugging info
    // printf_begin();             // needed only once for printing details
    // radio.printDetails();       // (smaller) function that prints raw register values
    // radio.printPrettyDetails(); // (larger) function that prints human readable data
}

void loop()
{
    uint8_t pipe;
    if (radio.available(&pipe))
    {
        uint8_t bytes = radio.getPayloadSize();
        radio.read(&payload, bytes);
        Serial.print(F("Received "));
        Serial.print(bytes);
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe);
        Serial.print(F(": "));
        Serial.println(payload);

        // when we receive signal update the `ui` struct with our `pin` variable as index.
        display_update_ui(&main_ui, 0);
        // draw the ui with the updated fields.
        display_draw_ui(&main_ui);
    }
}