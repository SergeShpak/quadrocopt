CC := gcc
BUILD_FOLDER := build
BIN_FOLDER := bin
OUT := tests
OUT_PATH := $(BIN_FOLDER)/$(OUT)
UNITY_FOLDER := ../../Unity/src
UNITY_FILE_NAME := unity
UNITY_SRC := $(UNITY_FILE_NAME).c
UNITY_OBJ := $(UNITY_FILE_NAME).o
UNITY_SRC_PATH := $(UNITY_FOLDER)/$(UNITY_SRC)
UNITY_BUILD_PATH := $(BUILD_FOLDER)/$(UNITY_OBJ)
TESTS_OBJ := utils_tests.o
ALL_OBJ := $(TESTS_OBJ) $(UNITY_OBJ)
PREFIXED_OBJ := $(addprefix $(BUILD_FOLDER)/, $(ALL_OBJ))
CFLAGS := -c

all: post-build

post-build: main-build

main-build: unity-build
	$(MAKE) build

unity-build: pre-build
	$(CC) $(CFLAGS) -o $(UNITY_BUILD_PATH) $(UNITY_SRC_PATH)

pre-build:
	mkdir -p $(BUILD_FOLDER)
	mkdir -p $(BIN_FOLDER)

build: $(TESTS_OBJ)
	$(CC) -o $(OUT_PATH) $(PREFIXED_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -o $(BUILD_FOLDER)/$@ $<


.PHONY: clean

clean:
	-rm -rf $(BUILD_FOLDER)
	-rm -rf $(BIN_FOLDER)
