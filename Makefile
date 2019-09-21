#生成文件名
TARGET=usart
#交叉编译器
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
RM=rm -rf
CORE=3
#--specs=nosys.specs 提供libnosys.a库 支持_read _write _isatty _sbrk _lseek，e.g printf，scanf，malloc等函数支持
CPUFLAGS=--specs=nosys.specs -mthumb -mcpu=cortex-m$(CORE) 
#链接器配置
LDFLAGS=-T stm32_flash.ld -Wl,-cref,-u,Reset_Handler -Wl,-Map=$(TARGET).map -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x80 -Wl,--start-group -lc -lm -Wl,--end-group
CFLAGS=$(INCFLAGS) -D STM32F10X_HD -D USE_STDPERIPH_DRIVER -Wall -g
#openocd配置
INTERFAGE_CFG=/usr/share/openocd/scripts/interface/stlink-v2.cfg
TAREGT_CFG=/usr/share/openocd/scripts/target/stm32f1x.cfg
PWD=$(shell pwd)

#头文件搜锁路径
INCFLAGS=-I $(PWD)/cmsis -I $(PWD)/libraries/inc -I $(PWD)/user \
-I $(PWD)/system/delay -I $(PWD)/system/sys -I $(PWD)/system/usart \
-I $(PWD)/hardware/KEY -I $(PWD)/hardware/LED -I $(PWD)/hardware/SPI -I $(PWD)/hardware/W25Q128

C_SRC=$(shell find ./ -name '*.c')
C_OBJ=$(C_SRC:%.c=%.o)

START_SRC = $(shell find ./ -name 'startup_stm32f10x_hd.s')
START_OBJ = $(START_SRC:%.s=%.o)

$(TARGET):$(START_OBJ) $(C_OBJ)
	$(CC) $^ $(CPUFLAGS) $(LDFLAGS) $(CFLAGS) -o $(TARGET).elf
$(START_OBJ):$(START_SRC)
	$(CC) -c $^ $(CPUFLAGS) $(CFLAGS) -o $@
$(C_OBJ):%.o:%.c
	$(CC) -c $^ $(CPUFLAGS) $(CFLAGS) -o $@
bin:
	$(OBJCOPY) $(TARGET).elf $(TARGET).bin
hex:
	$(OBJCOPY) $(TARGET).elf -Oihex $(TARGET).hex
clean:
	$(RM) $(shell find ./  -name '*.o')
cleanall:
	$(RM) $(shell find ./  -name '*.o') $(TARGET).*

download:
	openocd -f $(INTERFAGE_CFG) -f $(TAREGT_CFG) -c init -c halt -c "flash write_image erase $(PWD)/$(TARGET).elf" -C reset -c shutdown
