# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -pedantic

# Define the source files and the resulting binary files
WRITER_SRC = writer.c common.c
WRITER_OUT = writer.out

READER_SRC = reader.c common.c
READER_OUT = reader.out

# Default targets
all: $(WRITER_OUT) $(READER_OUT)

# Rule to compile writer.c and common.c
$(WRITER_OUT): $(WRITER_SRC)
	$(CC) $(CFLAGS) $(WRITER_SRC) -o $(WRITER_OUT)

# Rule to compile reader.c and common.c
$(READER_OUT): $(READER_SRC)
	$(CC) $(CFLAGS) $(READER_SRC) -o $(READER_OUT)

# Clean rule
clean:
	rm -f $(WRITER_OUT) $(READER_OUT)
