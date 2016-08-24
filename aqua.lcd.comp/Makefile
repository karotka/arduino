AVR_TOOLS_DIR = /opt/local
ARDUINO_DIR   = /Applications/Arduino.app/Contents/Java
ARDMK_DIR     = /Users/karotka/Arduino/make/
PROJECT_DIR   = /Users/karotka/Arduino
USER_LIB_PATH := $(PROJECT_DIR)/libraries
AVRDUDE_ISP_BAUDRATE = 115200

ISP_PORT      = /dev/tty.usbmodem1411
BOARD_TAG     = mega
MCU           = atmega2560

include /Users/karotka/Arduino/make/Arduino.mk

#flash:
#	/opt/local/bin/avrdude -q -v -p atmega2560 -C /opt/local/etc/avrdude.conf -D -c stk500v2 -b 115200 -P /dev/tty.usbmodem1411 -U flash:w:build-mega/aqua.lcd.comp.hex:i


