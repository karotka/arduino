AVR_TOOLS_DIR = /opt/local
ARDUINO_DIR   = /Applications/Arduino.app/Contents/Java
ARDMK_DIR     = /Users/karotka/Arduino/make/
PROJECT_DIR   = /Users/karotka/Arduino
USER_LIB_PATH := $(PROJECT_DIR)/libraries
AVRDUDE_ISP_BAUDRATE = 57600
CXXFLAGS = -Wl,-u,vfprintf -lprintf_flt -lm

ISP_PROG     = stk500v1
ISP_PORT      = /dev/tty.wchusbserial*
MCU           = atmega328p

include /Users/karotka/Arduino/make/Arduino.mk
