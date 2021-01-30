UPLOAD_PORT = /dev/tty.usbserial-14*
UPLOAD_SPEED = 460800
BOARD=d1_mini
BUILD_DIR=build
VERBOSE=1
FLASH_DEF=4M1M
FS_TYPE=spiffs
FS_DIR=datafs

$(shell minify data/nstyle.css > datafs/nstyle.css)
$(shell minify data/network_setup.html > datafs/network_setup.html)

include /Users/karotka/Arduino/make.esp/makeEspArduino.mk
