# --------------------------------------------------------------
# Wiegand-SPI fimreware for ATTINY25/45/85
#
# (c) 2016 Sam Thompson <git@samt.us>
# The MIT License
DEVICE     = attiny84
CLOCK      = 1000000
PROGRAMMER = -c wiegand -b 250000
OBJECTS    = main.o

# Use this to calcuate fuses: http://www.engbedded.com/fusecalc/
FUSES      = -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

AVRDUDE = avrdude $(PROGRAMMER) -C +avrdude.conf -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i
	gpio -g mode 9 ALT0
	gpio -g mode 10 ALT0
	gpio -g mode 11 ALT0

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f  main.elf main.hex main.o

main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
