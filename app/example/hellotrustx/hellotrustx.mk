NAME := hellotrustx

$(NAME)_SOURCES := hellotrustx.c

$(NAME)_COMPONENTS += mcu/esp32

GLOBAL_DEFINES += AOS_NO_WIFI

$(NAME)_COMPONENTS := yloop cli
$(NAME)_COMPONENTS += trustx

ifeq ($(BENCHMARKS),1)
$(NAME)_COMPONENTS  += benchmarks
GLOBAL_DEFINES      += CONFIG_CMD_BENCHMARKS
endif

GLOBAL_INCLUDES += ./