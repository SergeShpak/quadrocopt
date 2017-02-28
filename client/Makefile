CC=gcc
OUT=client
OUT_DEBUG=client-debug
OBJ=main.o calculate.o calculate_structs.o data_struct.o listener.o \
network_interactions.o nrutil.o sender.o threading_stuff.o utils.o \
io_stuff.o run_listener.o run_sender.o printer.o
PREF_OBJ=$(addprefix $(BUILD_FOLDER)/, $(OBJ))
BUILD_FOLDER=build
BIN_FOLDER=bin
CFLAGS=-c
LINK_FLAGS=-lm -pthread

all: CFLAGS += -o3
all: postbuild

debug: CFLAGS += -DDEBUG -g -pg -Wall
debug: LINK_FLAGS += -pg
debug: OUT = OUT_DEBUG
debug: postbuild 

postbuild: main-build

prebuild:
	mkdir -p $(BUILD_FOLDER)
	mkdir -p $(BIN_FOLDER)

main-build: prebuild
	@$(MAKE) --no-print-directory res

res: $(OBJ) 
	$(CC) $(PREF_OBJ) -o $(BIN_FOLDER)/$(OUT) $(LINK_FLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $(BUILD_FOLDER)/$@ $<

.PHONY: clean

clean:
	-rm -rf $(BUILD_FOLDER)
	-rm -rf $(BIN_FOLDER)
	-rm -f ./*.txt


