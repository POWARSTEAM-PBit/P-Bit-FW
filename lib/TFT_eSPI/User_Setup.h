#define USER_SETUP_INFO "ESP32 + ST7735 128x160"

#define ST7735_DRIVER
#define ST7735_ROBOTLCD  // For 128x160 RobotLCD
#define TFT_RGB_ORDER TFT_RGB
#define TFT_WIDTH  128
#define TFT_HEIGHT 160
#define TFT_INVERSION_OFF

// ==== ESP32 SPI Pins ====
#define TFT_MOSI 19
#define TFT_SCLK 25
#define TFT_CS   23
#define TFT_DC   22
#define TFT_RST  21
#define TFT_BL   -1  // no backlight pin
#define TOUCH_CS -1  // no touch screen pin (elimina warning de TFT_eSPI)

// ==== SPI Frequency ====
#define SPI_FREQUENCY 27000000
#define SPI_READ_FREQUENCY 20000000

// ==== Fonts ====
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT
