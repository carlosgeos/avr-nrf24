# User defined values
TARGETNAME = main
MCU:=atmega328p
F_CPU:=8000000
WORKDIR:=.

# Type: "avrdude -c ?" to get a full listing.
AVRDUDE_PROGRAMMER := usbtiny
# com1 = serial port. Use lpt1 to connect to parallel port.
AVRDUDE_PORT := usb

# Replace standard build tools by avr tools
CC = avr-gcc
AR  = @avr-ar

# Compile all .c files in all directories
SRC = $(shell ls $(WORKDIR)/*.c 2>/dev/null)

# Flags for the linker and the compiler
COMMON_FLAGS = -DF_CPU=$(F_CPU) -mmcu=$(MCU) $(DOGDEFS)
COMMON_FLAGS += -I$(WORKDIR) -I$(U8GM2DIR)
COMMON_FLAGS += -g -Os -Wall -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
COMMON_FLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
COMMON_FLAGS += -Wl,--relax -mcall-prologues
CFLAGS = $(COMMON_FLAGS) -std=gnu99 -Wstrict-prototypes

OBJ = $(SRC:.c=.o)

# Targets
.PHONY: all
all: $(TARGETNAME).hex
	avr-size $(TARGETNAME).elf

.PHONY: upload
upload: $(TARGETNAME).hex
	avrdude -F -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -v -v -U flash:w:$(TARGETNAME).hex
	avr-size $(TARGETNAME).elf

.PHONY: clean
clean:
	$(RM) $(TARGETNAME).hex $(TARGETNAME).elf $(TARGETNAME).a $(OBJ)

# implicit rules
%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom $< $@

# explicit rules
$(TARGETNAME).elf: $(TARGETNAME).a($(OBJ))
	$(LINK.o) $(COMMON_FLAGS) $(TARGETNAME).a $(LOADLIBES) $(LDLIBS) -o $@
