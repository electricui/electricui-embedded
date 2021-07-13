# AFL Makefile builds a minimal eUI program which accepts bytes from stdin, and then fuzzes it

AFL_ROOT = ~/tools/afl-2.52b
AFL_INPUTS = fuzz-inputs
AFL_OUTPUTS = fuzz-outputs

CLEANUP = rm -rf

#C_COMPILER = gcc
C_COMPILER = $(AFL_ROOT)/afl-gcc
AFL_RUNNER = $(AFL_ROOT)/afl-fuzz

# Source paths
SOURCE_ROOT = ../../src

# Compiler Settings
CFLAGS =  -std=c99
CFLAGS += -Wall
CFLAGS += -O2

TARGET = afl-eui.out

SRC_FILES = \
  afl-eui.c \
  $(SOURCE_ROOT)/electricui.c \
  $(SOURCE_ROOT)/transports/eui_binary_transport.c \
  $(SOURCE_ROOT)/eui_utilities.c

INC_DIRS = -I../src
SYMBOLS =

# AFL Settings
FUZZFLAGS = 

# Actions

default: clean build

all: clean build fuzz

build:
	$(C_COMPILER) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) -o $(TARGET)
	mkdir -p $(AFL_OUTPUTS)

clean:
	$(CLEANUP) $(TARGET) $(AFL_OUTPUTS)/*

fuzz:
	$(AFL_RUNNER) -i $(AFL_INPUTS) -o $(AFL_OUTPUTS) ./$(TARGET)