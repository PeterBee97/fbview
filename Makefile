CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lSDL2

TARGET = fbview
SRC = fbview.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) 