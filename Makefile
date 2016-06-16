all: Snake.hex

Snake.elf: Snake.c LCD.c
	avr-gcc -std=c99 -Wall -Wextra -mmcu=atmega324a -DF_CPU=16000000 -Os -o $@ $^

Snake.hex: Snake.elf
	avr-objcopy -j .text -j .data -O ihex $^ $@
	avr-size $^

clean:
	rm -rf Snake.hex Snake.elf

.PHONY: all clean