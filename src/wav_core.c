#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "wav_core.h"

uint16_t lire_u16_le(const uint8_t* p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

uint32_t lire_u32_le(const uint8_t* p) {
    return (uint32_t)(p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

void ecrire_u16_le(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

void ecrire_u32_le(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

int32_t abs_i32(int32_t x) {
    return (x < 0) ? -x : x;
}

int32_t borne_i32(int32_t x, int32_t min_v, int32_t max_v) {
    if (x < min_v) {
        return min_v;
    }
    if (x > max_v) {
        return max_v;
    }
    return x;
}

int32_t max_amp(uint16_t bits_per_sample) {
    if (bits_per_sample == 8u) {
        return 127;
    }
    if (bits_per_sample == 16u) {
        return 32767;
    }
    return 0;
}

int32_t lire_echantillon(const uint8_t* p, uint16_t bits_per_sample) {
    if (bits_per_sample == 8u) {
        return (int32_t)p[0] - 128;
    }
    if (bits_per_sample == 16u) {
        int16_t v = (int16_t)lire_u16_le(p);
        return (int32_t)v;
    }
    return 0;
}

void ecrire_echantillon(uint8_t* p, uint16_t bits_per_sample, int32_t v) {
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

int wav_rebuild_pcm_bytes(WavAudio* self, const WavFormat* fmt, const uint8_t* data, uint32_t data_size) {
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

    if (self->fmt.audio_format != 1u && self->fmt.audio_format != 6u && self->fmt.audio_format != 7u) {
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
