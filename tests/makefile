# ==========================================
#   ElectricUI Unit Testing
#   Uses Unity, www.throwtheswitch.org/unity as the base testing framework
# ==========================================

#Detect host OS
ifeq ($(OS),Windows_NT)
  ifeq ($(shell uname -s),) # not in a bash-like shell
	CLEANUP = del /F /Q
	MKDIR = mkdir
	DOWNLOADER  = echo "TODO downloader on win"
	EXTRACT = echo "TODO extraction on win"
  else # in a bash-like shell, like msys
	CLEANUP = rm -f
	MKDIR = mkdir -p
	DOWNLOADER = wget -nc
	EXTRACT = unzip -n
  endif
	TARGET_EXTENSION=.exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	DOWNLOADER = wget -nc
	EXTRACT = unzip -n
	TARGET_EXTENSION = .out
endif

C_COMPILER = gcc
ifeq ($(shell uname -s), Darwin)
C_COMPILER = clang
endif

#Unity download strings
UNITY_URL   = https://github.com/ThrowTheSwitch/Unity/archive/master.zip
UNITY_ROOT  = Unity-master

#Source paths
SOURCE_ROOT = ..
TEST_ROOT   = test
RUNNER_ROOT = runners

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

TARGET_BASE1 = eui_tests
TARGET1 = $(TARGET_BASE1)$(TARGET_EXTENSION)
SRC_FILES1 = \
  $(UNITY_ROOT)/src/unity.c \
  $(UNITY_ROOT)/extras/fixture/src/unity_fixture.c \
  $(SOURCE_ROOT)/electricui.c \
  $(SOURCE_ROOT)/eui_serial_transport.c \
  $(TEST_ROOT)/test_electricui.c \
  $(TEST_ROOT)/test_electricui_findobject.c \
  $(TEST_ROOT)/test_eui_serial_transport_crc16.c \
  $(TEST_ROOT)/test_eui_serial_transport_encoder.c \
  $(TEST_ROOT)/test_eui_serial_transport_decoder.c \
  $(RUNNER_ROOT)/runner_electricui.c \
  $(RUNNER_ROOT)/runner_eui_serial_transport.c \
  $(RUNNER_ROOT)/eui_tests.c
INC_DIRS = -Isrc -I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src
SYMBOLS =

default:
	$(C_COMPILER) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES1) -o $(TARGET1)

run:
	./$(TARGET1)

clean:
	$(CLEANUP) $(TARGET1)

all: clean default run

download: 
	$(DOWNLOADER) $(UNITY_URL)
	$(EXTRACT) master.zip

first: download default run

ci: CFLAGS += -Werror
ci: default
