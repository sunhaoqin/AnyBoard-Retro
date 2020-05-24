set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_PRIV_INCLUDEDIRS lib/lcd lib/sdcard)

set(COMPONENT_SRCS
	lib/lcd/lcd.c
	lib/sdcard/spi_sdcard.c
	periph_lcd.c 
	periph_spi_sdcard.c
	)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES esp_peripherals)
set(COMPONENT_PRIV_REQUIRES ab_driver board fatfs)

register_component()