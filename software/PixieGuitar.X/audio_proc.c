#include "audio_proc.h"

#include <dsp.h>

#include "analog.h"

static uint16_t const pitch_bounds[PITCH_COUNT + 1] = {
#include "bucket_freq.inl"
};

static uint8_t const gains[PITCH_COUNT] = {
#include "gains.inl"
};

static fractcomplex twiddle[ANALOG_BUFFER_LEN / 2] __attribute__((space(xmemory)));
static fractcomplex complex_buf[ANALOG_BUFFER_LEN] __attribute__((space(ymemory)));
static fractional   window[ANALOG_BUFFER_LEN]      __attribute__((space(ymemory)));

static int16_t Sat(int32_t x) {
  if (x > 0x7FFF) return 0x7FFF;
  if (x < 0) return 0;
  return x;
}

static inline int32_t Sum(int16_t const * p, unsigned count) {
  int32_t result = 0;
  // TODO: this is better done in assembly, using the accumulator.
  while (count--) {
    result += *p++;
  }
  return result;
}

static int16_t Trapezoid(uint8_t w, int16_t a, int16_t b) {
  return  (((int32_t) a + b) * w) >> 9;
}

static int16_t SumBucket(int16_t const * in, unsigned index) {
    uint16_t lo = pitch_bounds[index];
    uint16_t hi = pitch_bounds[index + 1];

    unsigned lo_int = lo >> 8;
    unsigned hi_int = hi >> 8;
    uint8_t lo_frc = lo & 0xFF;
    uint8_t hi_frc = hi & 0xFF;

    unsigned count = hi_int - lo_int;
    int16_t const * p = in + lo_int;
    int32_t result = Sum(p, count);
    result -= in[lo_int] / 2;
    result -= Trapezoid(lo_frc, in[lo_int], in[lo_int + 1]);

    result += in[hi_int] / 2;
    result += Trapezoid(hi_frc, in[hi_int], in[hi_int + 1]);

    return Sat(result);
}

static void Prebucket(int16_t const * in,
                      int16_t * out) {
  for (unsigned i = 0; i < PITCH_COUNT; ++i) {
    out[i] = SumBucket(in, i);
  }
}

static void BucketFull(int16_t const * in, int16_t * out) {
  for (unsigned i = 0; i < BUCKET_COUNT; ++i) {
    out[i] = Sum(in, 4);
    in += 4;
  }
}

static void BucketOctave(int16_t const * in, int16_t * out) {
  for (unsigned i = 0; i < BUCKET_COUNT; ++i) {
    int32_t sum = 0;
    unsigned j = i;
    for (unsigned octave = 0; octave < 4; ++octave) {
      sum += in[j];
      j += BUCKET_COUNT;
    }
    out[i] = Sat(sum);
  }
}

static void Bucket(int16_t const * in,
                   int16_t * scratch,
                   int16_t * out_full,
                   int16_t * out_octave) {
  Prebucket(in, scratch);

  if (out_full) BucketFull(scratch, out_full);
  if (out_octave) BucketOctave(scratch, out_octave);
}

void AudioProcInit() {
  TwidFactorInit(ANALOG_LOG2_BUFFER_LEN, twiddle, 0);
  HanningInit(ANALOG_BUFFER_LEN, window);
}

void AudioProcAnalyzePitch(int16_t * samples,
                           uint8_t gain,
                           int16_t ** raw_buckets,
                           int16_t ** full_buckets,
                           int16_t ** octave_buckets) {
  // Apply a window.
  VectorWindow(ANALOG_BUFFER_LEN, samples, samples, window);

  // Apply gain and convert to complex.
  for (unsigned i = 0; i < ANALOG_BUFFER_LEN; ++i) {
    complex_buf[i].real = samples[i] * gain;
    complex_buf[i].imag = 0;
  }

  // In-place FFT.
  FFTComplexIP(ANALOG_LOG2_BUFFER_LEN, complex_buf, twiddle, COEFFS_IN_DATA);
  BitReverseComplex(ANALOG_LOG2_BUFFER_LEN, complex_buf);

  // Compute magnitude (first half of the buffer).
  SquareMagnitudeCplx(ANALOG_BUFFER_LEN / 2, complex_buf, samples);

  // Harmonic suppression.
  for (int i = ANALOG_BUFFER_LEN / 2 - 1; i >= 0; --i) {
    samples[i] -= samples[i / 2] / 2;
    if (samples[i] < 0) samples[i] = 0;
  }

  // Apply bucketing.
  *raw_buckets = samples + ANALOG_BUFFER_LEN / 2;
  *full_buckets = *raw_buckets + PITCH_COUNT;
  *octave_buckets = *full_buckets + BUCKET_COUNT;
  Bucket(samples, *raw_buckets, *full_buckets, *octave_buckets);
}

uint16_t AudioProcPower(int16_t const * samples) {
  uint32_t total = 0;
  unsigned i = ANALOG_BUFFER_LEN;
  while (i--) {
    int16_t sample = *samples++;
    // TODO: Can optimize significantly using MAC.
    total += ((int32_t) sample * sample) >> 15;
  }
  return total >> ANALOG_LOG2_BUFFER_LEN;
}

char const * const full_bucket_names[BUCKET_COUNT] = {
  "E1-F#1", "F#1-G#1", "G#1-A#1", "A#1-C2", "C2-D2", "D2-E2",
  "E2-F#2", "F#2-G#2", "G#2-A#2", "A#2-C3", "C3-D3", "D3-E3",
  "E3-F#3", "F#3-G#3", "G#3-A#3", "A#3-C4", "C4-D4", "D4-E4",
};

char const * const octave_bucket_names[BUCKET_COUNT] = {
  "E", "E+", "F", "F+",
  "F#", "F#+", "G", "G+",
  "G#", "G#+", "A", "A+",
  "A#", "A#+", "B", "B+",
  "C", "C+", "C#", "C#+",
  "D", "D+", "D#", "D#+"
};
