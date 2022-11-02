TARGET = 351Files

# DEVICE ?= RG353P
# DEVICE ?= RG351P
# DEVICE ?= RG351V
# DEVICE ?= RG351MP
# DEVICE ?= RGB10
# DEVICE ?= RK2020
# DEVICE ?= CHI

DEVICE = RG353P
START_PATH = /storage
RES_PATH = /usr/share/351files/res
CC = g++
SDL2_CONFIG = sdl2-config

SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst %cpp,%o,$(SRC))
COMPILER_FLAGS =  $(shell $(SDL2_CONFIG) --cflags) -Wall -pedantic -Wfatal-errors -DDEVICE_$(DEVICE) -DSTART_PATH=\"$(START_PATH)\" -DRES_PATH=\"$(RES_PATH)\"
LINKER_FLAGS = $(shell $(SDL2_CONFIG) --libs) -lSDL2_image -lSDL2_ttf

all : $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LINKER_FLAGS)

%.o:%.cpp
	$(CC) -c $< -o $@ $(COMPILER_FLAGS)

clean :
	rm -f $(OBJ) $(TARGET)
