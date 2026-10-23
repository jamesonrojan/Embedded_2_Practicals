##########################################################################################################################
# EEE3096S 2026 - Practical 2A
#
# Generic gcc Makefile, same style as Practical 1A.
#
# In STM32CubeIDE this project is imported exactly as the practical brief says:
#   File -> Import -> C/C++ -> Existing Code as Makefile Project
#   (select this folder, toolchain: MCU ARM GCC)
# after which the hammer icon runs this Makefile.
#
# Command line:   make        build   -> build/Practical2_A.elf
#                 make clean
#
# Choose which task runs with #define RUN_TASK in Core/Src/main.c.
##########################################################################################################################

TARGET = Practical2_A

DEBUG = 1
OPT = -Og
BUILD_DIR = build

######################################
# sources
######################################
C_SOURCES =  \
Core/Src/main.c \
Core/Src/board_io.c \
Core/Src/task1_gpio.c \
Core/Src/task2_spi_config.c \
Core/Src/task3_spi_transfer.c \
Core/Src/task4_eeprom.c \
Core/Src/task5_fault_case.c \
Core/Src/task6_fsm.c \
Core/Src/stm32f0xx_it.c \
Core/Src/system_stm32f0xx.c \
Core/Src/stm32f0xx_hal_msp.c \
Core/Src/syscalls.c \
Core/Src/sysmem.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dma.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_exti.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash_ex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.c

# There is deliberately no SPI HAL driver in this project: the practical does
# not allow HAL_SPI_* for the SPI interface.

ASM_SOURCES = Core/Startup/startup_stm32f051c8tx.s

#######################################
# toolchain
#######################################
PREFIX = arm-none-eabi-

# STM32CubeIDE puts its GNU toolchain on the PATH when it runs this Makefile.
# From a plain terminal, if arm-none-eabi-gcc is not on PATH, fall back to the
# toolchain inside a standard STM32CubeIDE install, or pass GCC_PATH=...
ifndef GCC_PATH
ifeq ($(shell command -v $(PREFIX)gcc 2>/dev/null),)
GCC_PATH := $(firstword $(wildcard \
  /Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin \
  $(HOME)/st/stm32cubeide*/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin \
  /opt/st/stm32cubeide*/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin))
endif
endif

ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# flags
#######################################
MCU = -mcpu=cortex-m0 -mthumb

C_DEFS = -DSTM32F051x8 -DUSE_HAL_DRIVER

C_INCLUDES =  \
-ICore/Inc \
-IDrivers/STM32F0xx_HAL_Driver/Inc \
-IDrivers/STM32F0xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32F0xx/Include \
-IDrivers/CMSIS/Include

CFLAGS  = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections
ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

LDSCRIPT = STM32F051C8Tx_FLASH.ld
LIBS = -lc -lm -lnosys
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBS) \
          -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

#######################################
# build
#######################################
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

clean:
	-rm -fR $(BUILD_DIR)

-include $(wildcard $(BUILD_DIR)/*.d)
