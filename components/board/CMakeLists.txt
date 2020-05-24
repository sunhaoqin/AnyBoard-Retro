set(COMPONENT_ADD_INCLUDEDIRS include)

set(COMPONENT_SRCS board.c board_pins_config.c)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES audio_board driver esp_peripherals esp_dispatcher audio_hal)
set(COMPONENT_PRIV_REQUIRES audio_sal input_key_service ab_mem peripherals)

register_component()
