// soundtouch_wrapper.cpp
#include "libs/SoundTouch.h"

using namespace soundtouch;

extern "C" {
    SoundTouch* SoundTouch_create() {
        return new SoundTouch();
    }

    void SoundTouch_destroy(SoundTouch* soundTouch) {
        delete soundTouch;
    }

    void SoundTouch_setPitchSemiTones(SoundTouch* soundTouch, float pitchShift) {
        soundTouch->setPitchSemiTones(pitchShift);
    }

    void SoundTouch_putSamples(SoundTouch* soundTouch, const float* inBuffer, int numSamples) {
        soundTouch->putSamples(inBuffer, numSamples);
    }

    int SoundTouch_receiveSamples(SoundTouch* soundTouch, float* outBuffer, int maxSamples) {
        return soundTouch->receiveSamples(outBuffer, maxSamples);
    }

    void SoundTouch_flush(SoundTouch* soundTouch) {
        soundTouch->flush();
    }

    void SoundTouch_setSampleRate(SoundTouch* soundTouch, int sampleRate) {
        soundTouch->setSampleRate(sampleRate);
    }
    
    void SoundTouch_setChannels(SoundTouch* soundTouch, int channels) {
        soundTouch->setChannels(channels);
    }
}
