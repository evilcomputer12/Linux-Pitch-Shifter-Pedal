# Makefile for pitch_shifter project

CC = gcc
CFLAGS = -Wall -lm -lportaudio -lwiringPi -lwiringPiDev
TARGET = pitch_shifter
SRCS = main.c ssd1306_i2c.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
