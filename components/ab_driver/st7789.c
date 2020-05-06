/*********************
 *      INCLUDES
 *********************/
#include "st7789.h"
#include "board.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/



/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t data_bytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} st_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static esp_err_t st7789_send_cmd(const uint8_t cmd);
static esp_err_t st7789_send_data(const void * data, int length);

/**********************
 *  STATIC VARIABLES
 **********************/
static const st_init_cmd_t s_init_cmds[] = {
    {0x10, {0}, 0x80},

    {0x10, {0}, 0x80},

    {0x11, {0}, 0x80},

    /* Porch Setting */
    {0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
    /* Gate Control, Vgh=13.65V, Vgl=-10.43V */
    {0xB7, {0x35}, 1},
    /* VCOM Setting, VCOM=1.175V */
    {0xBB, {0x2B}, 1},
    /* LCM Control, XOR: BGR, MX, MH */
    {0xC0, {0x2C}, 1},
    /* VDV and VRH Command Enable, enable=1 */
    {0xC2, {0x01, 0xFF}, 2},
    /* VRH Set, Vap=4.4+... */
    {0xC3, {0x11}, 1},
    /* VDV Set, VDV=0 */
    {0xC4, {0x20}, 1},
    /* Frame Rate Control, 60Hz, inversion=0 */
    {0xC6, {0x0f}, 1},
    /* Power Control 1, AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V */
    {0xD0, {0xA4, 0xA1}, 2},

    {0x21, {0}, 0x80},
    /* Positive Voltage Gamma Control */
    {0xE0, {0xD0, 0x08, 0x11, 0x08, 0x0c, 0x15, 0x39, 0x33, 0x50, 0x36, 0x13, 0x14, 0x29, 0x29}, 14},
    /* Negative Voltage Gamma Control */
    {0xE1, {0xD0, 0x08, 0x10, 0x08, 0x06, 0x06, 0x39, 0x44, 0x51, 0x0b, 0x16, 0x14, 0x2f, 0x31}, 14},
    /* Memory Data Access Control, MX=MV=1, MY=ML=MH=0, RGB=0 */
    {0x36, {(1 << 7) | (1 << 5)}, 1},

    // {0xE4, {0x1D, 0x0, 0x10}, 3},
    /* Interface Pixel Format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x55}, 1},
    /* Porch Setting */
    /* Display On */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff}
};

static spi_device_handle_t s_spi_device = NULL;

// static spi_transaction_t s_spi_trans[8];

/**********************
 *      MACROS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

esp_err_t st7789_init(spi_device_handle_t spi_device) {
	s_spi_device = spi_device;

	gpio_set_direction(LCD_PIN_NUM_DC, GPIO_MODE_OUTPUT);

	//Send all the commands
	int i = 0;
	while (s_init_cmds[i].data_bytes != 0xff) {
		st7789_send_cmd(s_init_cmds[i].cmd);
		st7789_send_data(s_init_cmds[i].data, s_init_cmds[i].data_bytes & 0x1F);
		if (s_init_cmds[i].data_bytes & 0x80) {
			vTaskDelay(100 / portTICK_RATE_MS);
		}
		i++;
	}

	return ESP_OK;
}

esp_err_t st7789_deinit(void){
	st7789_send_cmd(0x28);
	vTaskDelay(100 / portTICK_RATE_MS);

	st7789_send_cmd(0x10);
	vTaskDelay(100 / portTICK_RATE_MS);

	return ESP_OK;
}

esp_err_t st7789_flush_locating(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
	uint8_t data[4];
	/*Column addresses*/
	st7789_send_cmd(0x2A);
	data[0] = (x1 >> 8) & 0xFF;
	data[1] = x1 & 0xFF;
	data[2] = (x2 >> 8) & 0xFF;
	data[3] = x2 & 0xFF;
	st7789_send_data(data, 4);

	/*Page addresses*/
	st7789_send_cmd(0x2B);
	data[0] = (y1 >> 8) & 0xFF;
	data[1] = y1 & 0xFF;
	data[2] = (y2 >> 8) & 0xFF;
	data[3] = y2 & 0xFF;
	st7789_send_data(data, 4);

	/*Memory write*/
	st7789_send_cmd(0x2C);

	return ESP_OK;
}

esp_err_t st7789_flash_line(uint16_t *line, int width, int lineCount) {
	// st7789_send_cmd(0x3C);
	st7789_send_data(line, width * 2 * lineCount);
	return ESP_OK;
}

esp_err_t st7789_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	st7789_flush_locating(x1, y1, x2, y2);

	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);
	uint16_t color_swap = ((color >> 8) & 0xFF) | ((color & 0xFF) << 8);	/*It's a 8 bit SPI bytes need to be swapped*/
	uint16_t buf[ST7789_HOR_RES];

	uint32_t i;
	if(size < ST7789_HOR_RES) {
		for(i = 0; i < size; i++) buf[i] = color_swap;

	} else {
		for(i = 0; i < ST7789_HOR_RES; i++) buf[i] = color_swap;
	}

	while(size > ST7789_HOR_RES) {
		st7789_send_data(buf, ST7789_HOR_RES * 2);
		size -= ST7789_HOR_RES;
	}

	st7789_send_data(buf, size * 2);	/*Send the remaining data*/

	return ESP_OK;
}

esp_err_t st7789_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, void* color_map) {
	st7789_flush_locating(x1, y1, x2, y2);

	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);

	/*Byte swapping is required*/
	// uint32_t i;
	// uint8_t * color_u8 = (uint8_t *) color_map;
	// uint8_t color_tmp;
	// for(i = 0; i < size * 2; i += 2) {
	// 	color_tmp = color_u8[i + 1];
	// 	color_u8[i + 1] = color_u8[i];
	// 	color_u8[i] = color_tmp;
	// }


	while(size > ST7789_HOR_RES) {
		st7789_send_data((void*)color_map, ST7789_HOR_RES * 2);
		size -= ST7789_HOR_RES;
		color_map += 2 * ST7789_HOR_RES;
	}

	st7789_send_data((void*)color_map, size * 2);	/*Send the remaining data*/

	return ESP_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static esp_err_t st7789_send_cmd(const uint8_t cmd) {
	spi_transaction_t t = {
		.length = 8,
		.tx_buffer = &cmd,
		.user=(void*)0,  // D/C needs to be set to 0
	};

	return spi_device_transmit(s_spi_device, &t);
}

static esp_err_t st7789_send_data(const void * data, int length) {
    if (length == 0) return ESP_FAIL;             //no need to send anything

	spi_transaction_t t = {
		.length = length * 8,
		.tx_buffer = data,
		.user = (void*)1,  // D/C needs to be set to 1
	};

	return spi_device_transmit(s_spi_device, &t);
}
