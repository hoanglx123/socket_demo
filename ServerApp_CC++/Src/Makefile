## Compiler tooll
GCC := g++

## Configuration flag and libraries
CFLAGS = -g
LIBS   = -lpthread  

## Configure path
ROOT      := ../
OUT_DIR   := $(ROOT)
TARGET    := $(OUT_DIR)/ServerApp
INC_DIR   := ../Inc
SRC_DIR   := .
OBJ_DIR   := .
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

INCLUDED  += -I$(INC_DIR)

################################################
########### Don't edit below ###################
################################################
## Build Rule
all: $(TARGET)

## Rule for making the target 
$(TARGET): $(OBJ_FILES)
	@echo "Linking the target..."
	@$(GCC) -o $@ $^ $(CFLAGS) $(LIBS)
	@echo "...successfully!!!"

## Rule for compiling the objects
$(OBJ_FILES) : $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	@echo "Compiling $<..."
	@$(GCC) -c $< -o $@ $(CFLAGS) $(LIBS) $(INCLUDED)
	@echo "...successfully!!!"

## Clean Rule
clean:
	@echo "Clean the project ..."
	@rm -f *.o $(TARGET)
	@echo "...successfully!!!"

.PHONY: all clean

	