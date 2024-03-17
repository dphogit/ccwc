# Define the compiler
CC = gcc

# Define C compiler flags (including warnings)
CFLAGS = -Wall

# Define source and object file paths
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Define source and object files
SRC = $(SRCDIR)/main.c
OBJ = $(OBJDIR)/main.o

# Define the final executable
EXE = ccwc
TARGET = $(BINDIR)/$(EXE)

# Main rule - builds the executable
all: $(TARGET)

# Rule to compile a specific source file to object file
$(OBJ): $(SRC)
	@mkdir -p $(OBJDIR)	# Create obj dir if not exist
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to link object files into the executable
$(TARGET): $(OBJ)
	@mkdir -p $(BINDIR)	# Create bin dir if not exist
	$(CC) $(CFLAGS) -o $@ $^

# Phony target for cleaning up object files
clean:
	rm -rf $(OBJ) $(OBJDIR)

