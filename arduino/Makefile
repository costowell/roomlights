
MAKE_DIR   := $(PWD)
#
# ----- setup wor Wemos D1 mini -----
#FQBN       ?= esp8266:esp8266:d1_mini
#IOT_NAME   ?= esp8266-meteo
#OTA_PORT   ?= 8266
#OTA_PASS   ?=
# ----- setup for Arduino Uno
FQBN        ?= arduino:avr:uno
# ----- ---------------------
V          ?= 0
VFLAG      =

ifeq "$(V)" "1"
VFLAG      =-v
endif

ifndef SERIAL_DEV
  ifneq (,$(wildcard /dev/ttyUSB0))
    SERIAL_DEV = /dev/ttyUSB0
  else ifneq (,$(wildcard /dev/ttyACM0))
    SERIAL_DEV = /dev/ttyACM0
  else
    SERIAL_DEV = unknown
  endif
endif

BUILD_DIR  := $(subst :,.,build/$(FQBN))

SRC        := $(wildcard *.ino)
HDRS       := $(wildcard *.h)
BIN        := $(BUILD_DIR)/$(SRC).bin
ELF        := $(BUILD_DIR)/$(SRC).elf

$(info FQBN       is [${FQBN}])
$(info IOT_NAME   is [${IOT_NAME}])
$(info OTA_PORT   is [${OTA_PORT}])
$(info OTA_PASS   is [${OTA_PASS}])
$(info V          is [${V}])
$(info VFLAG      is [${VFLAG}])
$(info MAKE_DIR   is [${MAKE_DIR}])
$(info BUILD_DIR  is [${BUILD_DIR}])
$(info SRC        is [${SRC}])
$(info HDRS       is [${HDRS}])
$(info BIN        is [${BIN}])
$(info SERIAL_DEV is [${SERIAL_DEV}])

all: $(ELF) upload
.PHONY: all

compile: $(ELF)
.PHONY: compile

$(ELF): $(SRC) $(HDRS)
	arduino-cli compile -b $(FQBN) $(VFLAG)
	@if which arduino-manifest.pl; \
	then echo "---> Generating manifest.txt"; \
	arduino-manifest.pl -b $(FQBN) $(SRC) $(HDRS) > manifest.txt; \
	else echo "---> If you want to generate manifest.txt, listing used libraries and their versions,"; \
	echo "---> please install arduino-manifest, see https://github.com/digiampietro/arduino-manifest"; \
	fi

upload:
	@if [ ! -c $(SERIAL_DEV) ] ; \
	then echo "---> ERROR: Serial Device not available, please set the SERIAL_DEV environment variable" ; \
	else echo -e "---> Uploading sketch\n"; \
	arduino-cli upload -b $(FQBN) -p $(SERIAL_DEV) $(VFLAG); \
	fi

monitor:
	@echo "---> Monitoring"
	@screen $(SERIAL_DEV)

clean:
	@echo "---> Cleaning the build directory"
	rm -rf build

