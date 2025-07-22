# Compiler and flags
CC = gcc

# Source files
SRC = optimized_main.c optimized_RGB_to_YCC.c optimized_YCC_to_RGB.c

# Output binary
BIN = CSC.out

# Default target: build the binary
all: $(BIN)

$(BIN): $(SRC)
	$(CC) -o $(BIN) $(SRC)

# Generate assembly files
asm:
	$(CC) -S optimized_main.c -o optimized_main.s
	$(CC) -S optimized_RGB_to_YCC.c -o optimized_RGB_to_YCC.s
	$(CC) -S optimized_YCC_to_RGB.c -o optimized_YCC_to_RGB.s

# Convert all .pgm files to .png (requires ImageMagick)
convert:
	for file in *.pgm; do convert "$$file" "$${file%.pgm}.png"; done

# Clean up all build outputs
clean:
	rm -f $(BIN) *.s *.png *.pgm
