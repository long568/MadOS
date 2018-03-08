export APP        = uip-test
export ARCH       = v7-m
export MCU_PREFIX = stm32f10x
export MCU_SUFFIX = cl
export TOOLCHAIN  = arm-none-eabi
export BUILD_VER  = debug

export AR    = $(TOOLCHAIN)-ar
export CC    = $(TOOLCHAIN)-gcc
export CPP   = $(TOOLCHAIN)-g++
export LD    = $(TOOLCHAIN)-ld
export OCPY  = $(TOOLCHAIN)-objcopy
export MAKE  = make
export ECHO  = @echo
export SET   = @set -e
export MKDIR = @mkdir -p
export RM    = @rm -f

export ROOT      = $(patsubst %/, %, $(shell pwd))
export LIBC_PATH = $(shell dirname $(shell dirname $(shell which $(CC))))/$(TOOLCHAIN)/lib
export LGCC_PATH = $(shell dirname $(shell dirname $(shell which $(CC))))/lib/gcc/$(TOOLCHAIN)/7.2.1/thumb/$(ARCH)
export BUILD_DIR = $(ROOT)/build
export TARGET    = $(BUILD_DIR)/$(APP)
export RULES     = $(ROOT)/rules.mk

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

export LIBS = -L$(LIBC_PATH) -L$(LGCC_PATH) -L$(BUILD_DIR)  \
              -ldrv -lkernel -larch -lm -lc -lgcc

ifeq ($(BUILD_VER), release)
DCMFLAGS =
DLDFLAGS = -s -x
else
DCMFLAGS = -g3
DLDFLAGS =
endif
CMFLAGS  = $(DEFS) $(INCS) $(DCMFLAGS) -std=c99 -Wall \
	       -march=arm$(ARCH) -mtune=cortex-m3 -mthumb-interwork \
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

rebuild: clean all

test:
	$(ECHO) $(shell dirname $(shell dirname $(shell which $(CC))))
