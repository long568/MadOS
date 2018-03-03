export APP  = uip-test
export ROOT = $(patsubst %/, %, $(shell pwd))
export MCU_PREFIX = stm32f10x
export MCU_SUFFIX = cl

export TOOLCHAIN = arm-none-eabi
export BUILD_VER = debug
export BUILD_DIR = $(ROOT)/build
export TARGET    = $(BUILD_DIR)/$(APP)
export PREBUILD  = $(BUILD_DIR)/$(APP)_PREBUILD

export DEFS = -DUSE_STDPERIPH_DRIVER \
			  -D$(shell echo $(MCU_PREFIX)_$(MCU_SUFFIX) | tr a-z A-Z)

export INCS = -I$(ROOT)/app/$(APP) \
			  -I$(ROOT)/app/$(APP)/inc \
              -I$(ROOT)/kernel/inc \
              -I$(ROOT)/kernel/lib/pt \
			  -I$(ROOT)/kernel/lib/timer \
			  -I$(ROOT)/library/uIP/uip \
			  -I$(ROOT)/library/uIP/uip-funs \
			  -I$(ROOT)/driver/$(MCU_PREFIX) \
			  -I$(ROOT)/arch/$(MCU_PREFIX)/Arch \
			  -I$(ROOT)/arch/$(MCU_PREFIX)/Startup \
			  -I$(ROOT)/arch/$(MCU_PREFIX)/StdPeriph \
			  -I$(ROOT)/arch/$(MCU_PREFIX)/StdPeriph/inc

export LIBS = -L$(BUILD_DIR) -ldrv -lkernel -larch

export MAKE  = make
export AR    = $(TOOLCHAIN)-ar
export CC    = $(TOOLCHAIN)-gcc
export CPP   = $(TOOLCHAIN)-g++
export LD    = $(TOOLCHAIN)-ld
export OCPY  = $(TOOLCHAIN)-objcopy
export ECHO  = @echo
export SET   = @set -e
export MKDIR = @mkdir -p
export RM    = @rm -f

ifeq ($(BUILD_VER), release)
DCMFLAGS =
DLDFLAGS = -s -x
else
DCMFLAGS = -g
DLDFLAGS =
endif
CMFLAGS  = $(DEFS) $(INCS) $(DCMFLAGS) -std=c99 -Wall \
	       -march=armv7-m -mtune=cortex-m3 -mthumb-interwork \
	       -nostdlib -O2
export LDFLAGS  += $(LIBS) $(DLDFLAGS) -nostdlib -Bstatic \
	               -T$(ROOT)/arch/$(MCU_PREFIX)/$(MCU_PREFIX)_$(MCU_SUFFIX).ld
export CFLAGS   += $(CMFLAGS)
export CPPFLAGS += $(CMFLAGS)

all :
	$(MAKE) -C $(ROOT)/arch/$(MCU_PREFIX)
	$(MAKE) -C $(ROOT)/driver/$(MCU_PREFIX)
	$(MAKE) -C $(ROOT)/kernel
	$(MAKE) -C $(ROOT)/library
	$(MAKE) -C $(ROOT)/app/$(APP)

clean :
	$(RM) -r $(BUILD_DIR)
	$(ECHO) 'Cleaning ... Done.'
