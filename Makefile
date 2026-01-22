# ===== Compiler & flags =====
CXX      := g++
CXXFLAGS := -Wall -Wextra
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
