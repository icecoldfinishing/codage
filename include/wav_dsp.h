#ifndef WAV_DSP_H
#define WAV_DSP_H

#include "wav_core.h"

int wav_downsample_by_2(const WavAudio* src, WavAudio* dst, bool use_max_pair);
int wav_quantize_16_to_8(const WavAudio* src, WavAudio* dst);
void wav_soft_desaturate_inplace(WavAudio* self, WavStats* out_stats);
void wav_normalize_inplace(WavAudio* self, double target_peak_ratio, WavStats* out_stats);
int wav_extract_left_channel(const WavAudio* src, WavAudio* dst);
int wav_stereo_to_2_1(const WavAudio* src, WavAudio* dst, bool apply_lfe_lowpass);
int wav_stereo_to_5_1(const WavAudio* src, WavAudio* dst);
int wav_generate_sine_stereo(WavAudio* dst, uint32_t sample_rate, uint16_t bits_per_sample, double duration_sec, double frequency_hz);
int wav_generate_sine_5_1_travel(WavAudio* dst, uint32_t sample_rate, uint16_t bits_per_sample, double duration_sec, double frequency_hz);

#endif
