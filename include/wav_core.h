#ifndef WAV_CORE_H
#define WAV_CORE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} WavFormat;

typedef struct {
    uint8_t* bytes;
    uint32_t byte_count;

    WavFormat fmt;
    uint32_t data_offset;
    uint32_t data_size;
} WavAudio;

typedef struct {
    uint32_t saturated_count;
    int32_t max_abs_before;
    int32_t max_abs_after;
    double applied_gain;
} WavStats;

// --- Utilitaires de bas niveau (Accesseurs binaires) ---
uint16_t lire_u16_le(const uint8_t* p);
uint32_t lire_u32_le(const uint8_t* p);
void ecrire_u16_le(uint8_t* p, uint16_t v);
void ecrire_u32_le(uint8_t* p, uint32_t v);
int32_t abs_i32(int32_t x);
int32_t borne_i32(int32_t x, int32_t min_v, int32_t max_v);
int32_t max_amp(uint16_t bits_per_sample);
int32_t lire_echantillon(const uint8_t* p, uint16_t bits_per_sample);
void ecrire_echantillon(uint8_t* p, uint16_t bits_per_sample, int32_t v);
int wav_rebuild_pcm_bytes(WavAudio* self, const WavFormat* fmt, const uint8_t* data, uint32_t data_size);

void wav_init(WavAudio* self);
void wav_free(WavAudio* self);
int wav_load_file(WavAudio* self, const char* path);
int wav_parse_header(WavAudio* self);
int wav_write_file(const WavAudio* self, const char* path);
int wav_clone(const WavAudio* src, WavAudio* dst);

void wav_print_info(const WavAudio* self);
void wav_print_stats(const char* label, const WavStats* stats);
int wav_play_file_simple(const char* path);

#endif
