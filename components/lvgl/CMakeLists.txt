execute_process(COMMAND git describe --always --tags --dirty)

set(COMPONENT_ADD_INCLUDEDIRS 
	lvgl 
	lvgl/src 
	lvgl/src/lv_core 
	lvgl/src/lv_draw 
	lvgl/src/lv_font 
	lvgl/src/lv_hal
	lvgl/src/lv_misc 
	lvgl/src/lv_objx 
	lvgl/src/lv_themes 
	lvgl_main
	lv_lib_lodepng
	)

set(COMPONENT_SRCDIRS 
	lvgl/src/lv_core 
	lvgl/src/lv_draw 
	lvgl/src/lv_font 
	lvgl/src/lv_hal 
	lvgl/src/lv_misc 
	lvgl/src/lv_objx 
	lvgl/src/lv_themes 
	lvgl_main 
	lv_lib_lodepng 
	)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES ab_driver ab_mem)
set(COMPONENT_PRIV_REQUIRES board peripherals input_key_service)

register_component()
