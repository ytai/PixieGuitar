#ifndef AUDIO_PROC_H
#define	AUDIO_PROC_H

#include <stdint.h>

#define BUCKET_COUNT 24
#define PITCH_COUNT  97

void AudioProcInit();

void AudioProcAnalyzePitch(int16_t * samples,
                           uint8_t gain,
                           int16_t ** raw_buckets,
                           int16_t ** full_buckets,
                           int16_t ** octave_buckets);

#endif  // AUDIO_PROC_H

