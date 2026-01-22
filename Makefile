# ===== Compiler & flags =====
CXX      := g++

BUILD ?= debug
ifeq ($(BUILD),debug)
	CXXFLAGS := -static -static-libgcc -static-libstdc++ -g3 -O0 -fno-omit-frame-pointer -fno-inline -fno-inline-functions -Wall -Wextra
else ifeq ($(BUILD),release)
	CXXFLAGS := -static -static-libgcc -static-libstdc++ -O3 -DNDEBUG
endif

LDFLAGS  :=

# ===== Directories =====
SRC_DIR   := src
OBJ_DIR   := obj
BUILD_DIR := build

# ===== Files =====
TARGET := $(BUILD_DIR)/Gluon.exe

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

# ===== Default target =====
all: $(TARGET)

# ===== Link =====
$(TARGET): $(OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

# ===== Compile =====
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ===== Utility =====
clean:
	rm -rf $(BUILD_DIR) $(OBJ_DIR)

.PHONY: all clean
