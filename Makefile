CC = gcc
CXX = g++
CFLAGS = -Wall -I./include -lm -lwiringPi -lportaudio

# Link libSoundTouch.a from the libs directory and the C++ standard library
LDFLAGS = ./libs/libSoundTouch.a -lstdc++

SRC = main.c soundtouch_wrapper.cpp ssd1306_i2c.c
OBJ = main.o soundtouch_wrapper.o ssd1306_i2c.o

TARGET = pitch_shifter

all: $(TARGET)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)