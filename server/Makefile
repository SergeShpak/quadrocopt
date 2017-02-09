CC=gcc
CFLAGS=-c -Wall -g
LINK_FLAGS=-lm
OUT=server
OBJ=main.o io_stuff.o data_struct.o calculate.o nrutil.o network_interactions.o utils.o
PREF_OBJ=$(addprefix $(BUILD_FOLDER)/, $(OBJ))
BUILD_FOLDER=build
BIN_FOLDER=bin

all: post-build

pre-build:
	mkdir -p $(BUILD_FOLDER)
	mkdir -p $(BIN_FOLDER)

main-build: pre-build
	@$(MAKE) --no-print-directory res

post-build: main-build

res: $(OBJ) 
	$(CC) $(PREF_OBJ) -o $(BIN_FOLDER)/$(OUT) $(LINK_FLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $(BUILD_FOLDER)/$@ $<

.PHONY: clean

clean:
	-rm -rf $(BUILD_FOLDER)
	-rm -rf $(BIN_FOLDER)
