CC=arm-none-eabi-gcc
TARGET=flash

MACH=-mcpu=cortex-m4
FLOAT=-mfloat-abi=hard

BUILD_DIR = Build
ROOT_DIR = ..
FREERTOS_DIR = ../../freertos

LINKER= -T $(ROOT_DIR)/linkers/STM32F411.ld
MAP_FILE= -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

CFLAGS= -c $(MACH) -mthumb $(FLOAT) -std=gnu99 -Wall -O0 -g \
		--specs=nano.specs -ffunction-sections -fdata-sections

LDFLAGS = $(MACH) -mthumb $(FLOAT) --specs=nosys.specs $(LINKER) $(MAP_FILE) -g \
			-Wl,--gc-sections -static --specs=nano.specs -Wl,--start-group -lc -lm \
			-Wl,--end-group -Wl,--print-memory-usage

OBJCOPY=arm-none-eabi-objcopy

###########################################
#				 INCLUDES
###########################################

INCLUDES+= -I $(ROOT_DIR)/app/Inc
INCLUDES+= -I $(ROOT_DIR)/drivers/Inc
INCLUDES+= -I $(ROOT_DIR)/common/Inc
INCLUDES+= -I $(ROOT_DIR)/interface/Inc

INCLUDES+= -I $(FREERTOS_DIR)/include
INCLUDES+= -I $(FREERTOS_DIR)/portable/GCC/ARM_CM4F

############################################
# 				SOURCE FILES
############################################

OBJS		+= $(BUILD_DIR)/main.o
OBJS		+= $(BUILD_DIR)/config.o
OBJS		+= $(BUILD_DIR)/syscalls.o
OBJS		+= $(BUILD_DIR)/startup.o

DRIVERS		+= $(BUILD_DIR)/driver_clock.o
DRIVERS		+= $(BUILD_DIR)/driver_systick.o
DRIVERS		+= $(BUILD_DIR)/driver_gpio.o
DRIVERS		+= $(BUILD_DIR)/driver_uart.o
DRIVERS		+= $(BUILD_DIR)/driver_interrupt.o
DRIVERS		+= $(BUILD_DIR)/driver_fpu.o

FREERTOS_OBJS += $(BUILD_DIR)/croutine.o
FREERTOS_OBJS += $(BUILD_DIR)/event_groups.o
FREERTOS_OBJS += $(BUILD_DIR)/list.o
FREERTOS_OBJS += $(BUILD_DIR)/queue.o
FREERTOS_OBJS += $(BUILD_DIR)/stream_buffer.o
FREERTOS_OBJS += $(BUILD_DIR)/tasks.o
FREERTOS_OBJS += $(BUILD_DIR)/timers.o

FREERTOS_OBJS += $(BUILD_DIR)/port.o
FREERTOS_OBJS += $(BUILD_DIR)/heap_4.o

PREREQ = $(BUILD_DIR)

all: $(PREREQ) $(BUILD_DIR)/$(TARGET).elf

$(PREREQ):
	mkdir $(PREREQ)

$(BUILD_DIR)/%.o: $(ROOT_DIR)/app/Src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$(*).o $(ROOT_DIR)/app/Src/$(*).c

$(BUILD_DIR)/%.o: $(ROOT_DIR)/drivers/Src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$(*).o $(ROOT_DIR)/drivers/Src/$(*).c

$(BUILD_DIR)/%.o: $(ROOT_DIR)/linkers/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$(*).o $(ROOT_DIR)/linkers/$(*).c

# FreeRTOS source rules
$(BUILD_DIR)/%.o: $(FREERTOS_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$(*).o $(FREERTOS_DIR)/$(*).c

$(BUILD_DIR)/%.o: $(FREERTOS_DIR)/portable/GCC/ARM_CM4F/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$(*).o $(FREERTOS_DIR)/portable/GCC/ARM_CM4F/$(*).c

$(BUILD_DIR)/%.o: $(FREERTOS_DIR)/portable/MemMang/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$(*).o $(FREERTOS_DIR)/portable/MemMang/$(*).c

# TARGET ELF
$(BUILD_DIR)/$(TARGET).elf: $(OBJS) $(DRIVERS) $(FREERTOS_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	$(OBJCOPY) -O binary $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin
	arm-none-eabi-size $(BUILD_DIR)/$(TARGET).elf

load:
	jlink flash.jlink

clean:
	rm -rf $(BUILD_DIR)/*.map $(BUILD_DIR)/*.o