#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include "resolution.h"

ReglePrefixe dictionnaire[] = {
    {'E', 0, 1},   
    {'A', 2, 2},   
    {'T', 6, 3},   
    {'N', 7, 3}    
};
int nb_regles = 4;

void conversion_base(uint16_t valeur, uint8_t base) {
    char chiffres[16];
    int i = 0;
    const char* alphabet = "0123456789ABCDEF";

    if (base < 2 || base > 16) {
        printf("Base invalide (%u). Utilisez une base entre 2 et 16.\n\n", base);
        return;
    }

    printf("Conversion dynamique de 0b");
    for (int b = 15; b >= 0; b--) printf("%d", (valeur >> b) & 1);

    uint16_t temp = valeur;
    while (temp > 0) {
        chiffres[i] = alphabet[temp % base];
        temp = temp / base;
        i++;
    }

    printf("\nResultat base %u: ", base);
    if (i == 0) printf("0");
    for (int j = i - 1; j >= 0; j--) {
        printf("%c", chiffres[j]);
    }
    printf("\n\n");
}




void decodage(uint8_t data) {
    printf("Exercice 1 : Decodage de 0x%02X\n", data);
    uint8_t registre = 0; 
    int len = 0;         
    printf("Sequence : ");
    // Parcourt les bits de l'octet (de gauche à droite)
    for (int i = 7; i >= 0; i--) {
        // Extrait le bit i et l'ajoute au registre
        uint8_t bit = (data >> i) & 1;
        registre = (registre << 1) | bit;
        len++;
        // Recherche une correspondance dans le dictionnaire
        for (int r = 0; r < nb_regles; r++) {
            if (len == dictionnaire[r].longueur && registre == dictionnaire[r].code) {
                printf("%c ", dictionnaire[r].lettre);
                registre = 0;
                len = 0;
                break;
            }
        }
    }
    printf("\n\n");
}

void encodage(const char* message) {
    printf("Exercice 1.3: Encodage dynamique de '%s'\n", message);
    uint8_t buffer = 0;
    int bits_utilises = 0;
    // On parcourt chaque lettre du message à encoder
    for (int i = 0; i < strlen(message); i++) {
        char lettre_cible = message[i];
        bool trouve = false;
        // On cherche la lettre dans le dictionnaire
        for (int r = 0; r < nb_regles; r++) {
            if (dictionnaire[r].lettre == lettre_cible) {
                trouve = true;
                uint8_t code = dictionnaire[r].code;
                int len = dictionnaire[r].longueur;
                // On insère les bits du code un par un dans le buffer
                for (int b = len - 1; b >= 0; b--) {
                    uint8_t bit = (code >> b) & 1; // Extraction du bit
                    buffer = (buffer << 1) | bit;  // Insertion dans le buffer
                    bits_utilises++;
                    // Si le buffer est plein (8 bits), on l'affiche et on reset
                    if (bits_utilises == 8) {
                        printf("Octet genere : 0x%02X\n", buffer);
                        buffer = 0;
                        bits_utilises = 0;
                    }
                }
                break; // Lettre trouvée et traitée, on passe à la suivante
            }
        }
        if (!trouve) printf("(Lettre '%c' inconnue du dictionnaire)\n", lettre_cible);
    }
    // Gestion du Padding : si le dernier octet n'est pas complet
    if (bits_utilises > 0) {
        // On décale les bits vers la gauche pour remplir l'octet (MSB align)
        uint8_t final_octet = buffer << (8 - bits_utilises);
        printf("Dernier octet (avec padding 0) : 0x%02X\n", final_octet);
    }
    printf("\n");
}

/**
 * @brief Compare deux tableaux via SIMD AVX2.
 * @details Remplace 32 itérations classiques par 1 seule comparaison vectorielle.
 * Res[i] = 0xFF si A[i] > B[i], sinon 0x00. [cite: 52, 53]
 */
void simd(const uint8_t* A, const uint8_t* B, uint8_t* Res, int n) {
    // On traite les données par blocs de 32 octets (256 bits) [cite: 48, 55]
    for (int i = 0; i <= n - 32; i += 32) {
        
        // 1. Chargement des données (LOD)
        // On utilise _mm256_loadu_si256 pour éviter un crash si l'adresse n'est pas alignée sur 32 octets [cite: 59, 60]
        __m256i vecA = _mm256_loadu_si256((const __m256i*)&A[i]);
        __m256i vecB = _mm256_loadu_si256((const __m256i*)&B[i]);

        // 2. Comparaison Vectorielle (CMP)
        // L'instruction _mm256_cmpgt_epi8 effectue 32 comparaisons 'A[i] > B[i]' en parallèle 
        __m256i masque_resultat = _mm256_cmpgt_epi8(vecA, vecB);

        // 3. Stockage (STR)
        _mm256_storeu_si256((__m256i*)&Res[i], masque_resultat);
    }
}

int localiser_premier_zero_simd(const uint8_t* ptr) {
    __m128i target = _mm_set1_epi8(0x00); 
    __m128i data = _mm_loadu_si128((const __m128i*)ptr); 
    __m128i cmp = _mm_cmpeq_epi8(data, target);
    int mask = _mm_movemask_epi8(cmp);
    if (mask == 0) return -1; 
    return __builtin_ctz(mask); 
}

int rechercher_caractere_simd(const uint8_t* ptr, uint8_t cible) {
    __m128i target = _mm_set1_epi8(cible); 
    __m128i data = _mm_loadu_si128((const __m128i*)ptr);
    __m128i cmp = _mm_cmpeq_epi8(data, target); 
    int mask = _mm_movemask_epi8(cmp);
    printf("Masque de recherche pour '%c' (0x%02X) : 0x%04X\n", cible, cible, mask);
    if (mask == 0) return -1;
    return __builtin_ctz(mask); 
}

static uint16_t masque_nb_bits(uint8_t nb_bits) {
    if (nb_bits >= 16) {
        return 0xFFFFu;
    }
    return (uint16_t)((1u << nb_bits) - 1u);
}

void convertisseur_init(ConvertisseurNumerique* self, uint8_t nb_bits_signed, double epsilon) {
    if (self == NULL) {
        return;
    }

    self->nb_bits_signed = (nb_bits_signed < 2 || nb_bits_signed > 16) ? 8 : nb_bits_signed;
    self->epsilon = (epsilon > 0.0) ? epsilon : 1e-6;
    self->buffer[0] = '\0';
}

uint16_t convertir_vers_signed_msb(const ConvertisseurNumerique* self, int16_t valeur) {
    if (self == NULL) {
        return 0;
    }

    uint8_t n = self->nb_bits_signed;
    uint8_t magnitude_bits = (uint8_t)(n - 1);
    uint16_t signe = (valeur < 0) ? 1u : 0u;

    int32_t abs_val = (valeur < 0) ? -(int32_t)valeur : (int32_t)valeur;
    uint16_t magnitude_max = (uint16_t)((1u << magnitude_bits) - 1u);
    if (abs_val > magnitude_max) {
        abs_val = magnitude_max;
    }

    uint16_t code = (uint16_t)((signe << magnitude_bits) | (uint16_t)abs_val);
    return (uint16_t)(code & masque_nb_bits(n));
}

int16_t convertir_depuis_signed_msb(const ConvertisseurNumerique* self, uint16_t code) {
    if (self == NULL) {
        return 0;
    }

    uint8_t n = self->nb_bits_signed;
    uint8_t magnitude_bits = (uint8_t)(n - 1);
    uint16_t masque = masque_nb_bits(n);
    uint16_t code_normalise = (uint16_t)(code & masque);

    uint16_t signe = (uint16_t)((code_normalise >> magnitude_bits) & 0x1u);
    uint16_t magnitude = (uint16_t)(code_normalise & ((1u << magnitude_bits) - 1u));

    return (signe == 1u) ? (int16_t)(-(int16_t)magnitude) : (int16_t)magnitude;
}

uint32_t encoder_ieee754_simple_precision(float valeur) {
    union {
        float f;
        uint32_t u;
    } conv;

    conv.f = valeur;
    return conv.u;
}

IEEE754Simple decomposer_ieee754_simple_precision(float valeur) {
    IEEE754Simple info;
    info.brut = encoder_ieee754_simple_precision(valeur);
    info.signe = (uint8_t)((info.brut >> 31) & 0x1u);
    info.exposant_biase = (uint8_t)((info.brut >> 23) & 0xFFu);
    info.mantisse = info.brut & 0x7FFFFFu;
    return info;
}

const char* ieee754_simple_precision_vers_binaire_1_8_23(float valeur, char* sortie, uint32_t taille_sortie) {
    if (sortie == NULL || taille_sortie < 36) {
        return NULL;
    }

    uint32_t brut = encoder_ieee754_simple_precision(valeur);
    uint32_t pos = 0;

    sortie[pos++] = (char)('0' + ((brut >> 31) & 0x1u));
    sortie[pos++] = '|';

    for (int i = 30; i >= 23; i--) {
        sortie[pos++] = (char)('0' + ((brut >> i) & 0x1u));
    }
    sortie[pos++] = '|';

    for (int i = 22; i >= 0; i--) {
        sortie[pos++] = (char)('0' + ((brut >> i) & 0x1u));
    }

    sortie[pos] = '\0';
    return sortie;
}

const char* entier_relatif_vers_base(const ConvertisseurNumerique* self, int32_t valeur, uint8_t base, char* sortie, uint32_t taille_sortie) {
    if (self == NULL || sortie == NULL || taille_sortie < 3) {
        return NULL;
    }
    if (base < 2 || base > 16) {
        snprintf(sortie, taille_sortie, "Base invalide");
        return sortie;
    }

    const char* alphabet = "0123456789ABCDEF";
    uint32_t pos = 0;
    uint32_t abs_val;

    if (valeur < 0) {
        sortie[pos++] = '-';
        abs_val = (uint32_t)(-(int64_t)valeur);
    } else {
        abs_val = (uint32_t)valeur;
    }

    char inverse[64];
    uint32_t len = 0;
    if (abs_val == 0u) {
        inverse[len++] = '0';
    } else {
        while (abs_val > 0u && len < (uint32_t)(sizeof(inverse) - 1u)) {
            inverse[len++] = alphabet[abs_val % base];
            abs_val /= base;
        }
    }

    for (int32_t i = (int32_t)len - 1; i >= 0; i--) {
        if (pos + 1 >= taille_sortie) {
            sortie[taille_sortie - 1] = '\0';
            return sortie;
        }
        sortie[pos++] = inverse[i];
    }

    sortie[pos] = '\0';
    return sortie;
}

const char* decimal_vers_binaire(const ConvertisseurNumerique* self, double valeur, char* sortie, uint32_t taille_sortie) {
    if (self == NULL || sortie == NULL || taille_sortie < 4) {
        return NULL;
    }

    if (isnan(valeur)) {
        snprintf(sortie, taille_sortie, "NaN");
        return sortie;
    }
    if (isinf(valeur)) {
        snprintf(sortie, taille_sortie, (valeur < 0) ? "-Inf" : "Inf");
        return sortie;
    }

    double x = valeur;
    uint32_t pos = 0;

    if (x < 0.0) {
        sortie[pos++] = '-';
        x = -x;
    }

    unsigned long long partie_entiere = (unsigned long long)floor(x);
    double partie_fractionnaire = x - (double)partie_entiere;

    char entier_inverse[96];
    uint32_t len_entier = 0;
    if (partie_entiere == 0ull) {
        entier_inverse[len_entier++] = '0';
    } else {
        while (partie_entiere > 0ull && len_entier < (uint32_t)(sizeof(entier_inverse) - 1u)) {
            entier_inverse[len_entier++] = (char)('0' + (partie_entiere & 1ull));
            partie_entiere >>= 1;
        }
    }

    for (int32_t i = (int32_t)len_entier - 1; i >= 0; i--) {
        if (pos + 1 >= taille_sortie) {
            sortie[taille_sortie - 1] = '\0';
            return sortie;
        }
        sortie[pos++] = entier_inverse[i];
    }

    if (partie_fractionnaire <= self->epsilon) {
        sortie[pos] = '\0';
        return sortie;
    }

    if (pos + 2 >= taille_sortie) {
        sortie[taille_sortie - 1] = '\0';
        return sortie;
    }

    sortie[pos++] = '.';

    uint32_t garde = 0;
    while (partie_fractionnaire > self->epsilon && garde < (taille_sortie - pos - 1u)) {
        partie_fractionnaire *= 2.0;
        if (partie_fractionnaire >= 1.0) {
            sortie[pos++] = '1';
            partie_fractionnaire -= 1.0;
        } else {
            sortie[pos++] = '0';
        }
        garde++;
    }

    sortie[pos] = '\0';
    return sortie;
}

static uint16_t lire_u16_le(const uint8_t* p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t lire_u32_le(const uint8_t* p) {
    return (uint32_t)(p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

static void ecrire_u16_le(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static void ecrire_u32_le(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static int32_t abs_i32(int32_t x) {
    return (x < 0) ? -x : x;
}

static int32_t borne_i32(int32_t x, int32_t min_v, int32_t max_v) {
    if (x < min_v) {
        return min_v;
    }
    if (x > max_v) {
        return max_v;
    }
    return x;
}

static int32_t max_amp(uint16_t bits_per_sample) {
    if (bits_per_sample == 8u) {
        return 127;
    }
    if (bits_per_sample == 16u) {
        return 32767;
    }
    return 0;
}

static int32_t lire_echantillon(const uint8_t* p, uint16_t bits_per_sample) {
    if (bits_per_sample == 8u) {
        return (int32_t)p[0] - 128;
    }
    if (bits_per_sample == 16u) {
        int16_t v = (int16_t)lire_u16_le(p);
        return (int32_t)v;
    }
    return 0;
}

static void ecrire_echantillon(uint8_t* p, uint16_t bits_per_sample, int32_t v) {
    if (bits_per_sample == 8u) {
        int32_t centered = borne_i32(v, -128, 127);
        p[0] = (uint8_t)(centered + 128);
        return;
    }
    if (bits_per_sample == 16u) {
        int32_t clamped = borne_i32(v, -32768, 32767);
        ecrire_u16_le(p, (uint16_t)(int16_t)clamped);
    }
}

static int wav_rebuild_pcm_bytes(WavAudio* self, const WavFormat* fmt, const uint8_t* data, uint32_t data_size) {
    if (self == NULL || fmt == NULL || data == NULL) {
        return -1;
    }

    uint32_t file_size = 44u + data_size;
    uint8_t* bytes = (uint8_t*)malloc(file_size);
    if (bytes == NULL) {
        return -1;
    }

    memcpy(bytes + 0, "RIFF", 4);
    ecrire_u32_le(bytes + 4, file_size - 8u);
    memcpy(bytes + 8, "WAVE", 4);

    memcpy(bytes + 12, "fmt ", 4);
    ecrire_u32_le(bytes + 16, 16u);
    ecrire_u16_le(bytes + 20, fmt->audio_format);
    ecrire_u16_le(bytes + 22, fmt->num_channels);
    ecrire_u32_le(bytes + 24, fmt->sample_rate);
    ecrire_u32_le(bytes + 28, fmt->byte_rate);
    ecrire_u16_le(bytes + 32, fmt->block_align);
    ecrire_u16_le(bytes + 34, fmt->bits_per_sample);

    memcpy(bytes + 36, "data", 4);
    ecrire_u32_le(bytes + 40, data_size);
    memcpy(bytes + 44, data, data_size);

    wav_free(self);
    self->bytes = bytes;
    self->byte_count = file_size;
    self->fmt = *fmt;
    self->data_offset = 44u;
    self->data_size = data_size;
    return 0;
}

void wav_init(WavAudio* self) {
    if (self == NULL) {
        return;
    }
    self->bytes = NULL;
    self->byte_count = 0;
    memset(&self->fmt, 0, sizeof(self->fmt));
    self->data_offset = 0;
    self->data_size = 0;
}

void wav_free(WavAudio* self) {
    if (self == NULL) {
        return;
    }
    free(self->bytes);
    self->bytes = NULL;
    self->byte_count = 0;
    memset(&self->fmt, 0, sizeof(self->fmt));
    self->data_offset = 0;
    self->data_size = 0;
}

int wav_load_file(WavAudio* self, const char* path) {
    if (self == NULL || path == NULL) {
        return -1;
    }

    FILE* f = fopen(path, "rb");
    if (f == NULL) {
        return -1;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }

    long file_size = ftell(f);
    if (file_size <= 0) {
        fclose(f);
        return -1;
    }

    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return -1;
    }

    uint8_t* buffer = (uint8_t*)malloc((size_t)file_size);
    if (buffer == NULL) {
        fclose(f);
        return -1;
    }

    size_t read_count = fread(buffer, 1, (size_t)file_size, f);
    fclose(f);
    if (read_count != (size_t)file_size) {
        free(buffer);
        return -1;
    }

    wav_free(self);
    self->bytes = buffer;
    self->byte_count = (uint32_t)file_size;
    return 0;
}

int wav_parse_header(WavAudio* self) {
    if (self == NULL || self->bytes == NULL || self->byte_count < 44u) {
        return -1;
    }

    if (memcmp(self->bytes + 0, "RIFF", 4) != 0 || memcmp(self->bytes + 8, "WAVE", 4) != 0) {
        return -1;
    }

    bool fmt_found = false;
    bool data_found = false;

    uint32_t pos = 12u;
    while (pos + 8u <= self->byte_count) {
        const uint8_t* chunk = self->bytes + pos;
        uint32_t chunk_size = lire_u32_le(chunk + 4);
        uint32_t chunk_data_pos = pos + 8u;
        if (chunk_data_pos > self->byte_count) {
            return -1;
        }

        if (chunk_data_pos + chunk_size > self->byte_count) {
            return -1;
        }

        if (memcmp(chunk, "fmt ", 4) == 0) {
            if (chunk_size < 16u) {
                return -1;
            }
            const uint8_t* fmt = self->bytes + chunk_data_pos;
            self->fmt.audio_format = lire_u16_le(fmt + 0);
            self->fmt.num_channels = lire_u16_le(fmt + 2);
            self->fmt.sample_rate = lire_u32_le(fmt + 4);
            self->fmt.byte_rate = lire_u32_le(fmt + 8);
            self->fmt.block_align = lire_u16_le(fmt + 12);
            self->fmt.bits_per_sample = lire_u16_le(fmt + 14);
            fmt_found = true;
        } else if (memcmp(chunk, "data", 4) == 0) {
            self->data_offset = chunk_data_pos;
            self->data_size = chunk_size;
            data_found = true;
        }

        pos = chunk_data_pos + chunk_size + (chunk_size & 1u);
    }

    if (!fmt_found || !data_found) {
        return -1;
    }

    if (self->fmt.audio_format != 1u) {
        return -1;
    }
    if (self->fmt.bits_per_sample != 8u && self->fmt.bits_per_sample != 16u) {
        return -1;
    }
    return 0;
}

int wav_write_file(const WavAudio* self, const char* path) {
    if (self == NULL || self->bytes == NULL || self->byte_count == 0u || path == NULL) {
        return -1;
    }

    FILE* f = fopen(path, "wb");
    if (f == NULL) {
        return -1;
    }

    size_t written = fwrite(self->bytes, 1, self->byte_count, f);
    fclose(f);
    return (written == self->byte_count) ? 0 : -1;
}

int wav_clone(const WavAudio* src, WavAudio* dst) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }

    uint8_t* copy = (uint8_t*)malloc(src->byte_count);
    if (copy == NULL) {
        return -1;
    }

    memcpy(copy, src->bytes, src->byte_count);
    wav_free(dst);
    dst->bytes = copy;
    dst->byte_count = src->byte_count;
    dst->fmt = src->fmt;
    dst->data_offset = src->data_offset;
    dst->data_size = src->data_size;
    return 0;
}

int wav_downsample_by_2(const WavAudio* src, WavAudio* dst, bool use_max_pair) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }

    uint16_t bps = src->fmt.bits_per_sample;
    uint16_t sample_bytes = (uint16_t)(bps / 8u);
    uint32_t frame_bytes = src->fmt.block_align;
    if (sample_bytes == 0u || frame_bytes == 0u) {
        return -1;
    }

    uint32_t frame_count = src->data_size / frame_bytes;
    uint32_t out_frames = frame_count / 2u;
    if (out_frames == 0u) {
        return -1;
    }

    uint32_t out_data_size = out_frames * frame_bytes;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    const uint8_t* data = src->bytes + src->data_offset;
    for (uint32_t i = 0; i < out_frames; i++) {
        const uint8_t* f1 = data + (2u * i) * frame_bytes;
        const uint8_t* f2 = f1 + frame_bytes;
        uint8_t* fout = out_data + i * frame_bytes;

        for (uint16_t c = 0; c < src->fmt.num_channels; c++) {
            const uint8_t* s1 = f1 + c * sample_bytes;
            const uint8_t* s2 = f2 + c * sample_bytes;
            int32_t v1 = lire_echantillon(s1, bps);
            int32_t v2 = lire_echantillon(s2, bps);
            int32_t mix = 0;

            if (use_max_pair) {
                mix = (abs_i32(v1) >= abs_i32(v2)) ? v1 : v2;
            } else {
                mix = (v1 + v2) / 2;
            }

            ecrire_echantillon(fout + c * sample_bytes, bps, mix);
        }
    }

    WavFormat out_fmt = src->fmt;
    out_fmt.sample_rate = (src->fmt.sample_rate > 1u) ? (src->fmt.sample_rate / 2u) : 1u;
    out_fmt.byte_rate = out_fmt.sample_rate * out_fmt.block_align;

    int rc = wav_rebuild_pcm_bytes(dst, &out_fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

int wav_quantize_16_to_8(const WavAudio* src, WavAudio* dst) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }
    if (src->fmt.bits_per_sample != 16u) {
        return -1;
    }

    uint32_t in_frame_bytes = src->fmt.block_align;
    uint32_t frame_count = src->data_size / in_frame_bytes;

    WavFormat out_fmt = src->fmt;
    out_fmt.bits_per_sample = 8u;
    out_fmt.block_align = out_fmt.num_channels;
    out_fmt.byte_rate = out_fmt.sample_rate * out_fmt.block_align;

    uint32_t out_data_size = frame_count * out_fmt.block_align;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    const uint8_t* in_data = src->bytes + src->data_offset;
    for (uint32_t i = 0; i < frame_count; i++) {
        const uint8_t* in_frame = in_data + i * in_frame_bytes;
        uint8_t* out_frame = out_data + i * out_fmt.block_align;

        for (uint16_t c = 0; c < src->fmt.num_channels; c++) {
            int16_t s16 = (int16_t)lire_u16_le(in_frame + c * 2u);
            int32_t u8 = ((int32_t)s16 + 32768) >> 8;
            out_frame[c] = (uint8_t)borne_i32(u8, 0, 255);
        }
    }

    int rc = wav_rebuild_pcm_bytes(dst, &out_fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

void wav_soft_desaturate_inplace(WavAudio* self, WavStats* out_stats) {
    if (self == NULL || self->bytes == NULL) {
        return;
    }

    uint16_t bps = self->fmt.bits_per_sample;
    uint16_t sb = (uint16_t)(bps / 8u);
    if (sb == 0u) {
        return;
    }

    int32_t peak = max_amp(bps);
    if (peak <= 0) {
        return;
    }

    WavStats stats;
    memset(&stats, 0, sizeof(stats));

    uint8_t* data = self->bytes + self->data_offset;
    uint32_t sample_count = self->data_size / sb;
    double tanh_norm = tanh(1.8);

    for (uint32_t i = 0; i < sample_count; i++) {
        uint8_t* p = data + i * sb;
        int32_t s = lire_echantillon(p, bps);
        int32_t a = abs_i32(s);
        if (a > stats.max_abs_before) {
            stats.max_abs_before = a;
        }
        if (a >= peak) {
            stats.saturated_count++;
        }

        double x = (double)s / (double)peak;
        double y = tanh(1.8 * x) / tanh_norm;
        int32_t out = (int32_t)llround(y * (double)peak);
        ecrire_echantillon(p, bps, out);

        int32_t ao = abs_i32(out);
        if (ao > stats.max_abs_after) {
            stats.max_abs_after = ao;
        }
    }

    stats.applied_gain = 1.0;
    if (out_stats != NULL) {
        *out_stats = stats;
    }
}

void wav_normalize_inplace(WavAudio* self, double target_peak_ratio, WavStats* out_stats) {
    if (self == NULL || self->bytes == NULL) {
        return;
    }

    uint16_t bps = self->fmt.bits_per_sample;
    uint16_t sb = (uint16_t)(bps / 8u);
    if (sb == 0u) {
        return;
    }

    int32_t peak = max_amp(bps);
    if (peak <= 0) {
        return;
    }

    if (target_peak_ratio <= 0.0 || target_peak_ratio > 1.0) {
        target_peak_ratio = 0.95;
    }

    WavStats stats;
    memset(&stats, 0, sizeof(stats));

    uint8_t* data = self->bytes + self->data_offset;
    uint32_t sample_count = self->data_size / sb;

    for (uint32_t i = 0; i < sample_count; i++) {
        int32_t s = lire_echantillon(data + i * sb, bps);
        int32_t a = abs_i32(s);
        if (a > stats.max_abs_before) {
            stats.max_abs_before = a;
        }
    }

    if (stats.max_abs_before == 0) {
        stats.applied_gain = 1.0;
        if (out_stats != NULL) {
            *out_stats = stats;
        }
        return;
    }

    double gain = ((double)peak * target_peak_ratio) / (double)stats.max_abs_before;
    stats.applied_gain = gain;

    for (uint32_t i = 0; i < sample_count; i++) {
        uint8_t* p = data + i * sb;
        int32_t s = lire_echantillon(p, bps);
        int32_t out = (int32_t)llround((double)s * gain);
        int32_t clamped = borne_i32(out, -peak, peak);
        if (clamped != out) {
            stats.saturated_count++;
        }
        ecrire_echantillon(p, bps, clamped);

        int32_t a = abs_i32(clamped);
        if (a > stats.max_abs_after) {
            stats.max_abs_after = a;
        }
    }

    if (out_stats != NULL) {
        *out_stats = stats;
    }
}

int wav_extract_left_channel(const WavAudio* src, WavAudio* dst) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }
    if (src->fmt.num_channels < 2u) {
        return -1;
    }

    uint16_t bps = src->fmt.bits_per_sample;
    uint16_t sb = (uint16_t)(bps / 8u);
    uint32_t in_fb = src->fmt.block_align;
    uint32_t frame_count = src->data_size / in_fb;

    WavFormat out_fmt = src->fmt;
    out_fmt.num_channels = 1u;
    out_fmt.block_align = sb;
    out_fmt.byte_rate = out_fmt.sample_rate * out_fmt.block_align;

    uint32_t out_data_size = frame_count * sb;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    const uint8_t* in_data = src->bytes + src->data_offset;
    for (uint32_t i = 0; i < frame_count; i++) {
        const uint8_t* in_frame = in_data + i * in_fb;
        memcpy(out_data + i * sb, in_frame, sb);
    }

    int rc = wav_rebuild_pcm_bytes(dst, &out_fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

int wav_stereo_to_2_1(const WavAudio* src, WavAudio* dst, bool apply_lfe_lowpass) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }
    if (src->fmt.num_channels != 2u) {
        return -1;
    }

    uint16_t bps = src->fmt.bits_per_sample;
    uint16_t sb = (uint16_t)(bps / 8u);
    uint32_t in_fb = src->fmt.block_align;
    uint32_t frame_count = src->data_size / in_fb;

    WavFormat out_fmt = src->fmt;
    out_fmt.num_channels = 3u;
    out_fmt.block_align = (uint16_t)(3u * sb);
    out_fmt.byte_rate = out_fmt.sample_rate * out_fmt.block_align;

    uint32_t out_data_size = frame_count * out_fmt.block_align;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    const uint8_t* in_data = src->bytes + src->data_offset;
    double lfe_mem = 0.0;

    for (uint32_t i = 0; i < frame_count; i++) {
        const uint8_t* in_frame = in_data + i * in_fb;
        uint8_t* out_frame = out_data + i * out_fmt.block_align;

        int32_t l = lire_echantillon(in_frame + 0u * sb, bps);
        int32_t r = lire_echantillon(in_frame + 1u * sb, bps);
        int32_t sub = (l + r) / 2;

        if (apply_lfe_lowpass) {
            lfe_mem = 0.92 * lfe_mem + 0.08 * (double)sub;
            sub = (int32_t)llround(lfe_mem);
        }

        ecrire_echantillon(out_frame + 0u * sb, bps, l);
        ecrire_echantillon(out_frame + 1u * sb, bps, r);
        ecrire_echantillon(out_frame + 2u * sb, bps, sub);
    }

    int rc = wav_rebuild_pcm_bytes(dst, &out_fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

int wav_stereo_to_5_1(const WavAudio* src, WavAudio* dst) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }
    if (src->fmt.num_channels != 2u) {
        return -1;
    }

    uint16_t bps = src->fmt.bits_per_sample;
    uint16_t sb = (uint16_t)(bps / 8u);
    uint32_t in_fb = src->fmt.block_align;
    uint32_t frame_count = src->data_size / in_fb;

    WavFormat out_fmt = src->fmt;
    out_fmt.num_channels = 6u;
    out_fmt.block_align = (uint16_t)(6u * sb);
    out_fmt.byte_rate = out_fmt.sample_rate * out_fmt.block_align;

    uint32_t out_data_size = frame_count * out_fmt.block_align;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    const uint8_t* in_data = src->bytes + src->data_offset;
    double lfe_mem = 0.0;

    for (uint32_t i = 0; i < frame_count; i++) {
        const uint8_t* in_frame = in_data + i * in_fb;
        uint8_t* out_frame = out_data + i * out_fmt.block_align;

        int32_t l = lire_echantillon(in_frame + 0u * sb, bps);
        int32_t r = lire_echantillon(in_frame + 1u * sb, bps);
        int32_t c = (l + r) / 2;
        lfe_mem = 0.94 * lfe_mem + 0.06 * (double)c;
        int32_t lfe = (int32_t)llround(lfe_mem);
        int32_t ls = l / 2;
        int32_t rs = r / 2;

        ecrire_echantillon(out_frame + 0u * sb, bps, l);
        ecrire_echantillon(out_frame + 1u * sb, bps, r);
        ecrire_echantillon(out_frame + 2u * sb, bps, c);
        ecrire_echantillon(out_frame + 3u * sb, bps, lfe);
        ecrire_echantillon(out_frame + 4u * sb, bps, ls);
        ecrire_echantillon(out_frame + 5u * sb, bps, rs);
    }

    int rc = wav_rebuild_pcm_bytes(dst, &out_fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

int wav_generate_sine_stereo(WavAudio* dst, uint32_t sample_rate, uint16_t bits_per_sample, double duration_sec, double frequency_hz) {
    if (dst == NULL) {
        return -1;
    }
    if ((bits_per_sample != 8u && bits_per_sample != 16u) || sample_rate == 0u || duration_sec <= 0.0 || frequency_hz <= 0.0) {
        return -1;
    }

    uint16_t channels = 2u;
    uint16_t sb = (uint16_t)(bits_per_sample / 8u);
    uint32_t frame_count = (uint32_t)llround(duration_sec * (double)sample_rate);
    if (frame_count == 0u) {
        return -1;
    }

    WavFormat fmt;
    fmt.audio_format = 1u;
    fmt.num_channels = channels;
    fmt.sample_rate = sample_rate;
    fmt.bits_per_sample = bits_per_sample;
    fmt.block_align = (uint16_t)(channels * sb);
    fmt.byte_rate = sample_rate * fmt.block_align;

    uint32_t out_data_size = frame_count * fmt.block_align;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    int32_t peak = max_amp(bits_per_sample);
    const double two_pi = 6.28318530717958647692;

    for (uint32_t n = 0; n < frame_count; n++) {
        double t = (double)n / (double)sample_rate;
        double s_l = sin(two_pi * frequency_hz * t);
        double s_r = sin(two_pi * frequency_hz * t + 0.25);

        int32_t l = (int32_t)llround(0.75 * s_l * (double)peak);
        int32_t r = (int32_t)llround(0.75 * s_r * (double)peak);

        uint8_t* frame = out_data + n * fmt.block_align;
        ecrire_echantillon(frame + 0u * sb, bits_per_sample, l);
        ecrire_echantillon(frame + 1u * sb, bits_per_sample, r);
    }

    int rc = wav_rebuild_pcm_bytes(dst, &fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

int wav_generate_sine_5_1_travel(WavAudio* dst, uint32_t sample_rate, uint16_t bits_per_sample, double duration_sec, double frequency_hz) {
    if (dst == NULL) {
        return -1;
    }
    if ((bits_per_sample != 8u && bits_per_sample != 16u) || sample_rate == 0u || duration_sec <= 0.0 || frequency_hz <= 0.0) {
        return -1;
    }

    uint16_t channels = 6u;
    uint16_t sb = (uint16_t)(bits_per_sample / 8u);
    uint32_t frame_count = (uint32_t)llround(duration_sec * (double)sample_rate);
    if (frame_count == 0u) {
        return -1;
    }

    WavFormat fmt;
    fmt.audio_format = 1u;
    fmt.num_channels = channels;
    fmt.sample_rate = sample_rate;
    fmt.bits_per_sample = bits_per_sample;
    fmt.block_align = (uint16_t)(channels * sb);
    fmt.byte_rate = sample_rate * fmt.block_align;

    uint32_t out_data_size = frame_count * fmt.block_align;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    int32_t peak = max_amp(bits_per_sample);
    const double two_pi = 6.28318530717958647692;

    for (uint32_t n = 0; n < frame_count; n++) {
        double t = (double)n / (double)sample_rate;
        double s = sin(two_pi * frequency_hz * t);
        double pos = ((double)n / (double)(frame_count - 1u)) * 5.0;
        uint8_t* frame = out_data + n * fmt.block_align;

        for (uint16_t ch = 0; ch < channels; ch++) {
            double d = fabs((double)ch - pos);
            double g = 1.0 - d;
            if (g < 0.0) {
                g = 0.0;
            }

            double amp = 0.85 * g * s;
            int32_t sample = (int32_t)llround(amp * (double)peak);
            ecrire_echantillon(frame + ch * sb, bits_per_sample, sample);
        }
    }

    int rc = wav_rebuild_pcm_bytes(dst, &fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

void wav_print_info(const WavAudio* self) {
    if (self == NULL) {
        return;
    }
    printf("WAV: sampleRate=%u Hz, channels=%u, bits=%u, byteRate=%u, blockAlign=%u\n",
           self->fmt.sample_rate,
           self->fmt.num_channels,
           self->fmt.bits_per_sample,
           self->fmt.byte_rate,
           self->fmt.block_align);
    printf("     dataOffset=%u, dataSize=%u octets\n", self->data_offset, self->data_size);
}

void wav_print_stats(const char* label, const WavStats* stats) {
    if (stats == NULL) {
        return;
    }
    printf("%s: sat=%u, maxBefore=%d, maxAfter=%d, gain=%.4f\n",
           (label != NULL) ? label : "Stats",
           stats->saturated_count,
           stats->max_abs_before,
           stats->max_abs_after,
           stats->applied_gain);
}

int wav_play_file_simple(const char* path) {
    if (path == NULL) {
        return -1;
    }

#if defined(_WIN32)
    char cmd[1024];
    int n = snprintf(cmd, sizeof(cmd), "powershell -NoProfile -Command \"(New-Object Media.SoundPlayer '%s').PlaySync()\"", path);
    if (n <= 0 || n >= (int)sizeof(cmd)) {
        return -1;
    }
    return system(cmd);
#else
    // Evite le message shell "ffplay: not found" dans les environnements minimaux.
    if (system("command -v ffplay >/dev/null 2>&1") != 0) {
        return -1;
    }

    char cmd[1024];
    int n = snprintf(cmd, sizeof(cmd), "ffplay -nodisp -autoexit -loglevel quiet \"%s\"", path);
    if (n <= 0 || n >= (int)sizeof(cmd)) {
        return -1;
    }
    return system(cmd);
#endif
}