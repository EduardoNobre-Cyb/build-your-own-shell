# Makefile for shell project

# Detect OS
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
    RM := del /Q
    RMDIR := rmdir /S /Q
    MKDIR := if not exist $@ mkdir $@
    FIXPATH = $(subst /,\,$1)
else
    DETECTED_OS := $(shell uname -s)
    RM := rm -f
    RMDIR := rm -rf
    MKDIR := mkdir -p $@
    FIXPATH = $1
endif

# Compiler settings
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -I.
LDFLAGS := -lreadline

# Directories
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin

# Target executables
TARGET := $(BIN_DIR)/shell
TARGET_ORIGINAL := $(BIN_DIR)/shell_original

# Source files for main version (refactored with CLI11)
SOURCES := shell.cpp \
           $(SRC_DIR)/path_utils.cpp \
           $(SRC_DIR)/command_parser.cpp \
           $(SRC_DIR)/command_executor.cpp \
           $(SRC_DIR)/builtins.cpp \
           $(SRC_DIR)/completion.cpp
OBJECTS := $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

# Source files for original monolithic version
SOURCES_ORIGINAL := shell_original.cpp
OBJECTS_ORIGINAL := $(SOURCES_ORIGINAL:%.cpp=$(BUILD_DIR)/%.o)

# Default target - build main version
.PHONY: all
all: $(TARGET)

# Build both versions
.PHONY: both
both: $(TARGET) $(TARGET_ORIGINAL)

# Build only original monolithic version
.PHONY: original
original: $(TARGET_ORIGINAL)

# Create necessary directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(BUILD_DIR)/$(SRC_DIR):
	@mkdir -p $(BUILD_DIR)/$(SRC_DIR)

# Link object files to create main executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo Build complete: $(TARGET)

# Link object files to create original monolithic executable
$(TARGET_ORIGINAL): $(OBJECTS_ORIGINAL) | $(BIN_DIR)
	$(CXX) $(OBJECTS_ORIGINAL) -o $@ $(LDFLAGS)
	@echo Build complete: $(TARGET_ORIGINAL)

# Compile source files to object files (from root directory)
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I. -c $< -o $@

# Compile source files to object files (from src directory)
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)/$(SRC_DIR)
	$(CXX) $(CXXFLAGS) -I. -c $< -o $@

# Clean build artifacts
.PHONY: clean
clean:
	@$(RMDIR) $(BUILD_DIR) 2>/dev/null || true
	@$(RMDIR) $(BIN_DIR) 2>/dev/null || true
	@echo Cleaned build artifacts

# Rebuild everything
.PHONY: rebuild
rebuild: clean all

# Run the main shell
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# Run the original monolithic shell
.PHONY: run-original
run-original: $(TARGET_ORIGINAL)
	./$(TARGET_ORIGINAL)

# Print help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all          - Build main shell version (default)"
	@echo "  both         - Build both main and original versions"
	@echo "  original     - Build only the original monolithic version"
	@echo "  clean        - Remove build artifacts"
	@echo "  rebuild      - Clean and rebuild"
	@echo "  run          - Build and run the main shell"
	@echo "  run-original - Build and run the original monolithic shell"
	@echo "  help         - Show this help message"
