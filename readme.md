# Wiegand AVR

This is firmware for ATTINY84/44 to decode a [Wiegand](https://en.wikipedia.org/wiki/Wiegand_interface) Transmission.

## Build

```bash
# copy sample avrdude
$ cp ./sample.avrdude.conf ./avrdude.conf

# (make edits to avrdude.conf to specify your programmer)

# build and flash
$ make 
$ make flash
```

## avrdude.conf

You will need to make some edits to avrdude.conf for `make flash` to work. 

## Usage

Once flashed to the ATTINY84/44, you may begin using it immediately.

1. Connect D0 and D1 from your wiegand reader to PA0 and PA1 respectively. 
2. Trigger a wiegand transmission from the reader by either scaning a card or using keypad
3. Note that PB2 will be high to indicate a data to be read
4. Using the SPI protocol, shift off 1 byte, this will be a non-zero number that indicates the numer of bits transmitted. You should have 1, 3, or 4 bytes indiciating a 4-bit keypad transmission, Wiegand26, or Wiegand 34 transmission respectively.
5. Divide the number you got in step 4 by 8 (with a floor of 1) to determine the number of bytes to read off.
6. The bytes are read back to you in the order they were received (MSB first)
