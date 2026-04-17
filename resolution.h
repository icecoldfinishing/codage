#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <immintrin.h>

#define BINAIRE_BUFFER_TAILLE 192

// Définition de la structure pour le dictionnaire dynamique
typedef struct {
    char lettre;
    uint8_t code;
    uint8_t longueur;
} ReglePrefixe;

// --- Exercice 0 : Bases ---
void conversion_base(uint16_t valeur, uint8_t base);

// --- Exercice 1 : Codage Variable ---
void decodage(uint8_t data);
void encodage(const char* message);

// --- Exercice 2 : Parallélisme SIMD ---
void simd(const uint8_t* A, const uint8_t* B, uint8_t* Res, int n);

// --- Exercice 3 : Recherche Rapide (Movemask) ---
int localiser_premier_zero_simd(const uint8_t* ptr);
int rechercher_caractere_simd(const uint8_t* ptr, uint8_t cible);

// --- Exercice 4 : Conversion OO (MSB signe, IEEE754, decimal->binaire) ---
typedef struct {
    uint8_t nb_bits_signed;
    double epsilon;
    char buffer[BINAIRE_BUFFER_TAILLE];
} ConvertisseurNumerique;

typedef struct {
    uint8_t signe;
    uint8_t exposant_biase;
    uint32_t mantisse;
    uint32_t brut;
} IEEE754Simple;

void convertisseur_init(ConvertisseurNumerique* self, uint8_t nb_bits_signed, double epsilon);
uint16_t convertir_vers_signed_msb(const ConvertisseurNumerique* self, int16_t valeur);
int16_t convertir_depuis_signed_msb(const ConvertisseurNumerique* self, uint16_t code);
uint32_t encoder_ieee754_simple_precision(float valeur);
IEEE754Simple decomposer_ieee754_simple_precision(float valeur);
const char* ieee754_simple_precision_vers_binaire_1_8_23(float valeur, char* sortie, uint32_t taille_sortie);
const char* decimal_vers_binaire(const ConvertisseurNumerique* self, double valeur, char* sortie, uint32_t taille_sortie);
const char* entier_relatif_vers_base(const ConvertisseurNumerique* self, int32_t valeur, uint8_t base, char* sortie, uint32_t taille_sortie);

// --- TP WAV : Manipulation Binaire OO ---
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

void wav_init(WavAudio* self);
void wav_free(WavAudio* self);
int wav_load_file(WavAudio* self, const char* path);
int wav_parse_header(WavAudio* self);
int wav_write_file(const WavAudio* self, const char* path);

int wav_clone(const WavAudio* src, WavAudio* dst);
int wav_downsample_by_2(const WavAudio* src, WavAudio* dst, bool use_max_pair);
int wav_quantize_16_to_8(const WavAudio* src, WavAudio* dst);
void wav_soft_desaturate_inplace(WavAudio* self, WavStats* out_stats);
void wav_normalize_inplace(WavAudio* self, double target_peak_ratio, WavStats* out_stats);
int wav_extract_left_channel(const WavAudio* src, WavAudio* dst);
int wav_stereo_to_2_1(const WavAudio* src, WavAudio* dst, bool apply_lfe_lowpass);
int wav_stereo_to_5_1(const WavAudio* src, WavAudio* dst);
int wav_generate_sine_stereo(WavAudio* dst, uint32_t sample_rate, uint16_t bits_per_sample, double duration_sec, double frequency_hz);
int wav_generate_sine_5_1_travel(WavAudio* dst, uint32_t sample_rate, uint16_t bits_per_sample, double duration_sec, double frequency_hz);

void wav_print_info(const WavAudio* self);
void wav_print_stats(const char* label, const WavStats* stats);
int wav_play_file_simple(const char* path);

#endif