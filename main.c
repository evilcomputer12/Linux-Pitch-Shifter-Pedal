#include <stdio.h> 
#include <stdlib.h>
#include <portaudio.h>
#include <math.h>
#include <signal.h>
#include <wiringPi.h>  // Include wiringPi for rotary encoder
#include <stdint.h>
#include "smbPitchShift.h"  // Include your pitch-shifting code
#include "ssd1306_i2c.h"
#include "soundtouch_wrapper.h"  // Include SoundTouch wrapper

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
#define FFT_FRAME_SIZE 1024  // Adjust this based on performance
#define outputA 5  // Rotary encoder A pin (WiringPi pin)
#define outputB 7  // Rotary encoder B pin (WiringPi pin)

volatile int counter = 0;  // Counter for logical clicks
unsigned int clickCount = 0;         // Counter for physical clicks
float pitchShiftRatio = 1.0f;        // Variable to hold pitch shift ratio

// Structure to store audio data between input and output
typedef struct {
    float pitchShift;
    long fftFrameSize;
    long osamp;
    float sampleRate;
    float inBuffer[FRAMES_PER_BUFFER];
    float outBuffer[FRAMES_PER_BUFFER];
    int stop;  // Flag to indicate when to stop
    SoundTouch *soundTouch;  // Add SoundTouch instance here
} pitchShiftData;

// Rotary encoder handling function
void updateSemitoneShift() {
    // Encoder routine. Updates counter if they are valid
  // and if rotated a full indent
  
    static uint8_t old_AB = 3;  // Lookup table index
    static int8_t encval = 0;   // Encoder value  
    static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table
    
    old_AB <<=2;  // Remember previous state  
    
    if (digitalRead(outputA)) old_AB |= 0x02; // Add current state of pin A
    if (digitalRead(outputB)) old_AB |= 0x01; // Add current state of pin B
    
    encval += enc_states[( old_AB & 0x0f )];
    
    // Update counter if encoder has rotated a full indent, that is at least 4 steps
    if( encval > 20 ) {        // Four steps forward
        counter++; // Increase counter
        if(counter > 12){
            counter = 12;
        }              
        encval = 0;
        delay(1);
    }
    else if( encval < -20 ) {  // Four steps backwards
        counter--; // Decrease counter
        if(counter < 0){
           counter = 0; 
        }               
        encval = 0;
        delay(1);
    }

    static int lastCounter = 0;

    if(counter != lastCounter){
        // Serial.println(counter);
        lastCounter = counter;
        // pitchShiftRatio = powf(2.0f, -(float)counter / 12.0f);
        char text[50];    // Buffer to hold the formatted string

        // Format the string with the semitone shift value
        snprintf(text, sizeof(text), "Semitones shift: %d\n", counter);

        // Optional: Clear the display before writing
        ssd1306_clearDisplay();
        ssd1306_setTextSize(2);
        // Display the formatted text on the OLED
        ssd1306_drawString(text);

        // Update the OLED display to show the new text
        ssd1306_display();
    }


    // Add a small delay to prevent excessive loop speed
    delay(1);
}

// Callback function for audio processing
static int pitchShiftCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo,
                              PaStreamCallbackFlags statusFlags, void *userData) {
    pitchShiftData *data = (pitchShiftData *)userData;
    float *in = (float *)inputBuffer;
    float *out = (float *)outputBuffer;

    // Check if the stop flag is set
    if (data->stop) {
        return paComplete;  // Signal to stop the stream
    }

    // Set the pitch shift ratio for the SoundTouch instance
    SoundTouch_setPitchSemiTones(data->soundTouch, -counter);
    
    // Process input samples through SoundTouch
    SoundTouch_putSamples(data->soundTouch, in, framesPerBuffer);
    int numReceived = SoundTouch_receiveSamples(data->soundTouch, out, framesPerBuffer);

    // Fill output buffer
    for (int i = 0; i < numReceived; ++i) {
        out[i] = out[i];  // Copy processed samples to output buffer
    }

    return paContinue;
}

// Main function
int main(void) {
    PaStream *stream;
    PaError err;
    pitchShiftData data;

    // Initialize WiringPi library
    if (wiringPiSetup() == -1) {
        printf("WiringPi initialization failed!\n");
        return 1;
    }

    // Set up rotary encoder pins
    pinMode(outputA, INPUT);
    pinMode(outputB, INPUT);
    pullUpDnControl(outputA, PUD_UP);
    pullUpDnControl(outputB, PUD_UP);

    ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
    ssd1306_clearDisplay();

    // Create SoundTouch instance and configure settings
    data.soundTouch = SoundTouch_create();
    SoundTouch_setSampleRate(data.soundTouch, SAMPLE_RATE);
    SoundTouch_setChannels(data.soundTouch, 1);  // Set to 1 for mono audio

    // Initialize pitch shift parameters
    data.pitchShift = pitchShiftRatio;  // Start with no pitch shift
    data.fftFrameSize = FFT_FRAME_SIZE;
    data.osamp = 4;  // Over-sampling factor
    data.sampleRate = SAMPLE_RATE;
    data.stop = 0;  // Initialize stop flag

    // Initialize PortAudio
    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    // Open audio stream for input and output
    err = Pa_OpenDefaultStream(&stream, 1, 1, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER, pitchShiftCallback, &data);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    // Start the audio stream
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    printf("Press Ctrl+C to stop...\n");

    // Run until the stop signal is received
    while (!data.stop) {
        updateSemitoneShift();  // Update semitone shift from rotary encoder
    }

    // Stop the audio stream
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    // Close the audio stream
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    // Terminate PortAudio
    Pa_Terminate();

    // Clean up SoundTouch instance
    SoundTouch_destroy(data.soundTouch);

    return 0;
}
