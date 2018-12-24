
NAME := trustx

ENABLE_DTLS := no

LIBTRUSTX := .

$(NAME)_TYPE := security

$(NAME)_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/
$(NAME)_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/pal
$(NAME)_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/common
$(NAME)_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/cmd
$(NAME)_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/comms
$(NAME)_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/ifx_i2c
$(NAME)_INCLUDES     += $(LIBTRUSTX)

GLOBAL_INCLUDES     += $(LIBTRUSTX)/..
GLOBAL_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/
GLOBAL_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/pal
GLOBAL_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/common
GLOBAL_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/cmd
GLOBAL_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/comms
GLOBAL_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/ifx_i2c
ifeq ($(ENABLE_DTLS), yes)
GLOBAL_INCLUDES     += $(LIBTRUSTX)/optiga/include/optiga/dtls
endif

$(NAME)_SOURCES     += 	$(LIBTRUSTX)/pal/alios/pal_gpio.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/pal/alios/pal_os_timer.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/pal/alios/pal_os_lock.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/pal/alios/pal_os_event.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/pal/alios/pal_ifx_i2c_config.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/pal/alios/pal_i2c.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/util/optiga_util.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/crypt/optiga_crypt.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/comms/optiga_comms.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/comms/ifx_i2c/ifx_i2c_config.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/comms/ifx_i2c/ifx_i2c.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/comms/ifx_i2c/ifx_i2c_physical_layer.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/comms/ifx_i2c/ifx_i2c_data_link_layer.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/comms/ifx_i2c/ifx_i2c_transport_layer.c

ifeq ($(ENABLE_DTLS), yes)
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/dtls/AlertProtocol.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/dtls/DtlsFlightHandler.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/dtls/DtlsHandshakeProtocol.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/dtls/DtlsRecordLayer.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/dtls/DtlsTransportLayer.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/dtls/DtlsWindowing.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/dtls/HardwareCrypto.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/dtls/MessageLayer.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/dtls/OCP.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/dtls/OCPConfig.c
endif

$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/common/Logger.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/common/Util.c
$(NAME)_SOURCES     +=  $(LIBTRUSTX)/optiga/cmd/CommandLib.c


