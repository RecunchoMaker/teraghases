OBJS =  $(patsubst %.c,%.o,$(wildcard *.c))
OBJS +=  $(patsubst src/%.c,src/%.o,$(wildcard src/*.c))
OBJS += $(patsubst target/%.c,target/%.o,$(wildcard target/*.c))
OBJS += $(patsubst printf/%.c,printf/%.o,$(wildcard printf/*.c))

OPENCM3_DIR	= ../../libopencm3

DEVICE          = stm32f103c8t6

CFLAGS          += -Os -ggdb3 -std=c99
CPPFLAGS        += -MD
LDFLAGS         += -static -nostartfiles
LDLIBS          += -Wl,--start-group -lm -lc -lgcc -lnosys -Wl,--end-group

include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(OPENCM3_DIR)/mk/gcc-config.mk

# caligari: modified link script to ust DFU bootloader 8K (ROM=0x8002000)
LDSCRIPT	= ../ld/stm32f103c8t6-dfu-8k.ld

.PHONY: clean all

all: binary.bin

clean:
	$(Q)$(RM) -rf binary.* *.o src/*.o src/*.d generated.*.ld

include $(OPENCM3_DIR)/mk/genlink-rules.mk
include $(OPENCM3_DIR)/mk/gcc-rules.mk

upload: binary.bin
# caligari: dfu uploader instead stlink
#	openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -c "program binary.hex verify reset exit"
	dfu-util --reset --alt 2 -D binary.bin	

