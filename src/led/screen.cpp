#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ARRAY_SIZE(x) ((sizeof x) / (sizeof *x))


#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32 
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C


#define BASE_TONE_BUZZER 220
#define DURATION_TONE_BUZZER 500
#define PIN_BUZZER 14

#define PIN_RADIO_D0 6
#define PIN_RADIO_D1 7
#define PIN_RADIO_D2 8
#define PIN_RADIO_D3 9

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

t_ui main_ui;

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
    for (size_t i = 0; i < 4; i++)
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

        // Serial.print(ui->rows[i].name);
        // Serial.print(":");
        // Serial.println(ui->rows[i].data);
    }
    // update display
    display.display();
}

void radio_setup()
{
    for (int i = 0; i < ARRAY_SIZE(RADIO_PINS); i++)
        pinMode(RADIO_PINS[i], INPUT);
}

t_pin radio_scan()
{
    while (1)
    {
        for (int i = 0; i < ARRAY_SIZE(RADIO_PINS); i++)
        {
            if (digitalRead(RADIO_PINS[i]))
                return (i);
        }
        delay(50);
    }
}

void setup()
{
    Serial.begin(115200);
    radio_setup();
    display_init_ui(&main_ui, PIN_NAMES);
    display_setup();

}

void loop()
{
    // blocking call keeps on scanning until we receive a signal.
    t_pin pin = radio_scan();
#ifdef BUZZER
    tone(PIN_BUZZER, BASE_TONE_BUZZER * (pin + 1) / 2, DURATION_TONE_BUZZER);
#endif

    // when we receive signal update the `ui` struct with our `pin` variable as index.
    display_update_ui(&main_ui, pin);
    // draw the ui with the updated fields.
    display_draw_ui(&main_ui);
    delay(200);
}