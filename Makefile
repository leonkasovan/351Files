TARGET = EnhancedFileManager

# DEVICE ?= RG353P
# DEVICE ?= RG351P
# DEVICE ?= RG351V
# DEVICE ?= RG351MP
# DEVICE ?= RGB10
# DEVICE ?= RK2020
# DEVICE ?= CHI

DEVICE = RG353P
START_PATH = /userdata
BIN_PATH = /userdata/roms/bin
RES_PATH = $(BIN_PATH)/FileManager
CC = aarch64-linux-gnu-g++
SDL2_CONFIG = pkg-config sdl2,SDL2_ttf,SDL2_image

SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst %cpp,%o,$(SRC))
COMPILER_FLAGS =  $(shell $(SDL2_CONFIG) --cflags) -Wall -pedantic -Wfatal-errors -DDEVICE_$(DEVICE) -DSTART_PATH=\"$(START_PATH)\" -DRES_PATH=\"$(RES_PATH)\" -DBIN_PATH=\"$(BIN_PATH)\"
LINKER_FLAGS = $(shell $(SDL2_CONFIG) --libs) -s -w

# Build setup for arm64 in WSL2
export PKG_CONFIG_PATH="/usr/lib/aarch64-linux-gnu/pkgconfig"
all : $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LINKER_FLAGS)

%.o:%.cpp
	$(CC) -c $< -o $@ $(COMPILER_FLAGS)

clean :
	rm -f $(OBJ) $(TARGET) FileManager.7z
