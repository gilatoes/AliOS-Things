NAME := hellotrustx

$(NAME)_SOURCES := hellotrustx.c

$(NAME)_COMPONENTS += mcu/esp32

GLOBAL_DEFINES += AOS_NO_WIFI

$(NAME)_COMPONENTS := yloop cli
$(NAME)_COMPONENTS += rhino/vfs
$(NAME)_COMPONENTS += trustx

ifeq ($(BENCHMARKS),1)
$(NAME)_COMPONENTS  += benchmarks
GLOBAL_DEFINES      += CONFIG_CMD_BENCHMARKS
endif

GLOBAL_INCLUDES += ./
GLOBAL_INCLUDES += ./../../../kernel/rhino/vfs/include/
GLOBAL_INCLUDES += ./../../../kernel/rhino/vfs/include/device/

GLOBAL_INCLUDES += ./../../../platform/mcu/esp32/bsp/include/driver/include/driver/
GLOBAL_INCLUDES += ./../../../platform/mcu/esp32/hal/
