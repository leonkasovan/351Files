#TARGET = 351Files
TARGET = EnhancedFileManager

# DEVICE ?= RG353P
# DEVICE ?= RG351P
# DEVICE ?= RG351V
# DEVICE ?= RG351MP
# DEVICE ?= RGB10
# DEVICE ?= RK2020
# DEVICE ?= CHI

DEVICE = RG353P
START_PATH = /storage/roms
RES_PATH = /usr/share/FileManager/res
#RES_PATH = res
CC = g++
SDL2_CONFIG = pkg-config sdl2,SDL2_ttf,SDL2_image

SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst %cpp,%o,$(SRC))
COMPILER_FLAGS =  $(shell $(SDL2_CONFIG) --cflags) -Wall -pedantic -Wfatal-errors -DDEVICE_$(DEVICE) -DSTART_PATH=\"$(START_PATH)\" -DRES_PATH=\"$(RES_PATH)\"
LINKER_FLAGS = $(shell $(SDL2_CONFIG) --libs)

all : $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LINKER_FLAGS)

%.o:%.cpp
	$(CC) -c $< -o $@ $(COMPILER_FLAGS)

clean :
	rm -f $(OBJ) $(TARGET)
