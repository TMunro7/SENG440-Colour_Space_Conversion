# Compiler and flags
CC = gcc

# Source files
SRC = optimized_main.c optimized_RGB_to_YCC.c optimized_YCC_to_RGB.c

# Output binary
BIN = CSC.out

# Default target: build the binary
all: $(BIN)

$(BIN): $(SRC)
	gcc -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a9 -O3 -o CSC.out optimized_main.c optimized_RGB_to_YCC.c optimized_YCC_to_RGB.c

# Generate assembly files
asm:
asm:
	gcc -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a9 -S optimized_main.c -o optimized_main.s
	gcc -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a9 -S optimized_RGB_to_YCC.c -o optimized_RGB_to_YCC.s
	gcc -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a9 -S optimized_YCC_to_RGB.c -o optimized_YCC_to_RGB.s

# Convert all .pgm files to .png (requires ImageMagick)
convert:
	for file in *.pgm; do convert "$$file" "$${file%.pgm}.png"; done

# Clean up all build outputs
clean:
	rm -f $(BIN) *.s *.png *.pgm
