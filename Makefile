# Makefile for CUDA NPP Image Processing Project

# Compiler
NVCC = nvcc

# Executable name
TARGET = batch_grayscale

# Source file
SRC = main.cpp

# Compiler flags
# -arch=sm_xx should be set to your GPU's compute capability.
NVCC_FLAGS = -O2 -arch=sm_75

# Library path - update this to your CUDA installation's lib/x64 directory
# LIB_PATH = -L"D:\Softwares\CUDA\v12.4\Development\lib\x64"
LIB_PATH = -L"$(CUDA_PATH)\lib\x64"

# Libraries to link
# NPP has several variants (i, s, c, etc.). nppi is for imaging.
LIBS = -lnppicc

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRC)
	$(NVCC) $(NVCC_FLAGS) $(SRC) -o $(TARGET) $(LIB_PATH) $(LIBS)

# Rule to clean up the project directory
clean:
	del /F /Q $(TARGET).exe 2>nul || (exit 0)

# Phony targets
.PHONY: all clean