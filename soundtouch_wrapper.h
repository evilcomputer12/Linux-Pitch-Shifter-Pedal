// soundtouch_wrapper.h
#ifndef SOUNDTOUCH_WRAPPER_H
#define SOUNDTOUCH_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SoundTouch SoundTouch;

SoundTouch* SoundTouch_create();
void SoundTouch_destroy(SoundTouch* soundTouch);
void SoundTouch_setPitchSemiTones(SoundTouch* soundTouch, float pitchShift);
void SoundTouch_putSamples(SoundTouch* soundTouch, const float* inBuffer, int numSamples);
int SoundTouch_receiveSamples(SoundTouch* soundTouch, float* outBuffer, int maxSamples);
void SoundTouch_flush(SoundTouch* soundTouch);
void SoundTouch_setSampleRate(SoundTouch* soundTouch, int sampleRate);
void SoundTouch_setChannels(SoundTouch* soundTouch, int channels);
#ifdef __cplusplus
}
#endif

#endif // SOUNDTOUCH_WRAPPER_H
