#include <math.h>
#include "lcd.h"
#include "board.h"
#include "st7789.h"

#include "esp_log.h"

#include "driver/ledc.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"

static const char *TAG = "LCD";

#define BACKLIGHT_DUTY_RESOLUTION LEDC_TIMER_10_BIT
#define BACKLIGHT_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define BACKLIGHT_CHANEL LEDC_CHANNEL_0

static spi_device_handle_t s_spi_device;

static esp_err_t backlight_init() {
    // (duty range is 0 ~ ((2**bit_num)-1)

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = BACKLIGHT_DUTY_RESOLUTION, // resolution of PWM duty
        .freq_hz = 10*1000,                           // frequency of PWM signal
        .speed_mode = BACKLIGHT_SPEED_MODE,           // timer mode
        .timer_num = LEDC_TIMER_0                     // timer index
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel    = BACKLIGHT_CHANEL,
        .duty       = 0,
        .gpio_num   = LCD_PIN_NUM_BCKL,
        .intr_type  = LEDC_INTR_FADE_END,
        .speed_mode = BACKLIGHT_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_0
    };

    ledc_channel_config(&ledc_channel);

    // //initialize fade service.
    // ledc_fade_func_install(0);

    // // duty range is 0 ~ ((2**bit_num)-1)
    // ledc_set_fade_with_time(BACKLIGHT_SPEED_MODE, BACKLIGHT_CHANEL, 0, 500);
    // ledc_fade_start(BACKLIGHT_SPEED_MODE, BACKLIGHT_CHANEL, LEDC_FADE_NO_WAIT);

    return ESP_OK;
}

static void spi_pre_transfer_callback(spi_transaction_t *t) {
    int dc = (int)t->user;
    gpio_set_level(LCD_PIN_NUM_DC, dc);
}

esp_err_t lcd_init() {
    spi_bus_config_t spi_cfg = {0};
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz=40*1000*1000,           //Clock out at 40 MHz
        .mode=0,                                //SPI mode 0
        .queue_size=1,
        .pre_cb= spi_pre_transfer_callback,
        .post_cb= NULL,
        .flags = SPI_DEVICE_NO_DUMMY,
    };
    get_spi_pins(&spi_cfg, &dev_cfg);
    //Initialize the SPI bus
    int dma_chan = 1;
    spi_bus_initialize(HSPI_HOST, &spi_cfg, dma_chan);
    //Attach the LCD to the SPI bus
    spi_bus_add_device(HSPI_HOST, &dev_cfg, &s_spi_device);

	//Initialize the LCD driver
	st7789_init(s_spi_device);

	backlight_init();
    lcd_set_brightness(100);
    	
    return ESP_OK;
}

esp_err_t lcd_set_brightness(int brightness){

    if(brightness >= 0 && brightness <= 100){
        int duty = (int) (brightness * pow(2, BACKLIGHT_DUTY_RESOLUTION)/100);
        ledc_set_duty(BACKLIGHT_SPEED_MODE, BACKLIGHT_CHANEL, duty);
        ledc_update_duty(BACKLIGHT_SPEED_MODE, BACKLIGHT_CHANEL);
        return ESP_OK;
    }

    return ESP_FAIL;
}

void lcd_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, void* color_map){
	st7789_flush(x1, y1, x2, y2, color_map);
}