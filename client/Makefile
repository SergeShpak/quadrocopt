CC=gcc
CFLAGS=-c -Wall -g
OUT=client
OBJ=echoClientUDP.o io_stuff.o data_struct.o calculate.o nrutil.o
PREF_OBJ=$(addprefix $(BUILD_FOLDER)/, $(OBJ))
BUILD_FOLDER=build
BIN_FOLDER=bin
LINK_FLAGS=-lm

all: postbuild

postbuild: main-build

prebuild:
	mkdir -p $(BUILD_FOLDER)
	mkdir -p $(BIN_FOLDER)

main-build: prebuild
	@$(MAKE) --no-print-directory res

res: $(OBJ) 
	$(CC) $(PREF_OBJ) -o $(BIN_FOLDER)/$(OUT) $(LINK_FLAGS)

dirs:
	mkdir -p log

%.o: %.c
	$(CC) $(CFLAGS) -o $(BUILD_FOLDER)/$@ $<

.PHONY: clean

clean:
	-rm -rf $(BUILD_FOLDER)
	-rm -rf $(BIN_FOLDER)
	-rm -f ./*.txt
