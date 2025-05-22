# Makefile

# Compiler and Linker
CC = gcc
CXX = g++
LD = g++ # Use gcc to drive the linker

# Executable Name
TARGET = autochess_game

# --- Directories ---
SRC_C_DIR = src
SRC_OBJ_DIR = src/obj
SRC_IMGUI_DIR = src/imgui
SRC_INTERFACE_DIR = src
INC_DIR = include
INC_GLAD_DIR = include/glad
INC_KHR_DIR = include/KHR
INC_OBJ_DIR = include/obj
INC_CGLM_DIR = include/cglm
INC_IMGUI_DIR = include/imgui
BUILD_DIR = . # Output .o files to root directory

# --- Include Flags ---
INCLUDE_FLAGS = -I$(INC_DIR) -I$(INC_GLAD_DIR) -I$(INC_KHR_DIR) -I$(INC_OBJ_DIR) -I$(INC_CGLM_DIR) -I$(INC_IMGUI_DIR)
# (Optional SDL include path)
# INCLUDE_FLAGS += -IC:/tools/msys64/mingw64/include/SDL2

# --- Compiler Flags ---
CFLAGS = -Wall -g $(INCLUDE_FLAGS) -std=c11
CXXFLAGS = -Wall -g $(INCLUDE_FLAGS) -std=c++11

# --- Linker Flags ---
LDFLAGS =
# (Optional SDL library path)
# LDFLAGS += -LC:/tools/msys64/mingw64/lib

# --- Libraries ---
LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lOpenGL32 -lm -lstdc++

# --- Source Files (.c) ---
# Added shader.c from src/
SRCS = $(wildcard $(SRC_C_DIR)/*.c) $(wildcard $(SRC_OBJ_DIR)/*.c)

# --- ImGui and Interface Source Files (.cpp) ---
# Include the ImGui core files and backends
IMGUI_SRCS = $(SRC_IMGUI_DIR)/imgui.cpp \
             $(SRC_IMGUI_DIR)/imgui_draw.cpp \
             $(SRC_IMGUI_DIR)/imgui_tables.cpp \
             $(SRC_IMGUI_DIR)/imgui_widgets.cpp \
             $(SRC_IMGUI_DIR)/imgui_impl_sdl2.cpp \
             $(SRC_IMGUI_DIR)/imgui_impl_opengl3.cpp

# Interface file
INTERFACE_SRCS = $(SRC_INTERFACE_DIR)/imgui_interface.cpp

# Combine all C++ sources
SRCS_CXX = $(IMGUI_SRCS) $(INTERFACE_SRCS)

# --- Object Files (.o) ---
OBJS_C = $(notdir $(patsubst %.c, %.o, $(SRCS)))
OBJS_CXX = $(notdir $(patsubst %.cpp, %.o, $(SRCS_CXX)))
OBJS = $(OBJS_C) $(OBJS_CXX) # Combine lists

# --- DEBUG ---
# $(info Source Files SRCS: $(SRCS))
# $(info Object Files OBJS: $(OBJS))
# --- END DEBUG ---

# Default target: build the executable
all: $(TARGET)

# --- Rule to link the executable ---
$(TARGET): $(OBJS)
	@echo "--- Linking target: $@ ---"
	@echo "Using object files: $^"
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)
	@echo "Successfully linked executable: $(TARGET)"

# --- Compilation Rules ---

# Rule for C sources in src/ and src/obj/
%.o: $(SRC_C_DIR)/%.c
	@echo "Compiling (C) $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

%.o: $(SRC_OBJ_DIR)/%.c
	@echo "Compiling (C) $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for ImGui C++ files
%.o: $(SRC_IMGUI_DIR)/%.cpp
	@echo "Compiling (CXX-ImGui) $< -> $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule for Interface C++ files
%.o: $(SRC_INTERFACE_DIR)/%.cpp
	@echo "Compiling (CXX-Interface) $< -> $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Phony targets
.PHONY: all clean run

# Target to clean up build files
clean:
	@echo "Cleaning build files..."
	rm -f $(TARGET) $(OBJS)
	@echo "Cleaned."

# Optional: Target to run the game
run: all
	./$(TARGET)