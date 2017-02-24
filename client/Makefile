CC=gcc
CFLAGS=-c -Wall -g
OUT=client
OBJ=main.o calculate.o calculate_structs.o data_struct.o listener.o \
	network_interactions.o nrutil.o sender.o threading_stuff.o utils.o \
	io_stuff.o run_listener.o run_sender.o
PREF_OBJ=$(addprefix $(BUILD_FOLDER)/, $(OBJ))
BUILD_FOLDER=build
BIN_FOLDER=bin
LINK_FLAGS=-lm -pthread

all: postbuild

postbuild: main-build

prebuild:
	mkdir -p $(BUILD_FOLDER)
	mkdir -p $(BIN_FOLDER)

main-build: prebuild
	@$(MAKE) --no-print-directory res

res: $(OBJ) 
	$(CC) $(ADD_LIB) $(PREF_OBJ) -o $(BIN_FOLDER)/$(OUT) $(LINK_FLAGS)

dirs:
	mkdir -p log

%.o: %.c
	$(CC) $(CFLAGS) -o $(BUILD_FOLDER)/$@ $<

.PHONY: clean

clean:
	-rm -rf $(BUILD_FOLDER)
	-rm -rf $(BIN_FOLDER)
	-rm -f ./*.txt
