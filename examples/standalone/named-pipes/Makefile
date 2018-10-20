CLEANUP = rm -f
TARGET_EXTENSION = .out

C_COMPILER = gcc
ifeq ($(shell uname -s), Darwin)
C_COMPILER = clang
endif

#Source paths
LIB_ROOT = ../../../src

#Compiler
CFLAGS =  -std=c99
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-align
CFLAGS += -Wwrite-strings
CFLAGS += -Wswitch-default
CFLAGS += -Wunreachable-code
CFLAGS += -Winit-self
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wno-unknown-pragmas
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
CFLAGS += -Wold-style-definition

TARGET_BASE1 = pipe-device
TARGET1 = $(TARGET_BASE1)$(TARGET_EXTENSION)
SRC_FILES = \
  named-pipes.c \
  $(LIB_ROOT)/electricui.c \
  $(LIB_ROOT)/utilities/eui_crc.c \
  $(LIB_ROOT)/utilities/eui_offset_validation.c \
  $(LIB_ROOT)/transports/eui_serial_transport.c
INC_DIRS = -I$(LIB_ROOT)
SYMBOLS = 

default:
	$(C_COMPILER) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) -o $(TARGET1)

run:
	./$(TARGET1)

clean:
	$(CLEANUP) $(TARGET1)

all: clean default run
