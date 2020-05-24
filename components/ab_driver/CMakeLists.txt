set(COMPONENT_ADD_INCLUDEDIRS include)

set(COMPONENT_SRCS ./st7789.c)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES driver)
set(COMPONENT_PRIV_REQUIRES board)

register_component()
