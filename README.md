
## Instructions

Before flashing the `led` code onto the led fixtures, you gotta setup the device `CURRENT_READING_PIPE` in `config.h` this will set the led fixture's address for the rf24 module.


Also since we're using both a 3v3 pro micro and a 5v make sure you have the right one selected in `platformio.ini` otherwise you risk soft bricking it. 

