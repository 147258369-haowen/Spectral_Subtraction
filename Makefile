# Compiler
CC = g++

# Compilation flags
CFLAGS = -Wall -O2 -std=c++11  

# Target executable
TARGET = audio_handle

# Source and header files
SRCS = audio_handle.cpp Audio.cpp kiss_fft.cpp kiss_fftr.cpp
HDRS = _kiss_fft_guts.h Audio.h kiss_fft.h kiss_fftr.h

# Object files (generated from .c files)
OBJS = $(SRCS:.c=.o)

# Rule to build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

# Rule to compile each source file into an object file
%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean command to remove object files and executable
clean:
	rm -f $(OBJS) $(TARGET)
