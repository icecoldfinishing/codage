#ifndef WAV_COMPRESSION_H
#define WAV_COMPRESSION_H

#include "wav_core.h"

int wav_compress_alaw(const WavAudio* src, WavAudio* dst);
int wav_decompress_alaw(const WavAudio* src, WavAudio* dst);
int wav_compress_mulaw(const WavAudio* src, WavAudio* dst);
int wav_decompress_mulaw(const WavAudio* src, WavAudio* dst);
int wav_dynamic_range_compress(const WavAudio* src, WavAudio* dst, double threshold_db, double ratio, double makeup_gain_db);

#endif
