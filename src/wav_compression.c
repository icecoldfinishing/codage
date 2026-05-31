#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "wav_compression.h"

static uint8_t linear_to_alaw(int16_t pcm_val) {
    int mask = 0;
    int seg = 0;
    if (pcm_val < 0) {
        pcm_val = -pcm_val;
        mask = 0xD5;
    } else {
        mask = 0x55;
    }
    if (pcm_val > 32767) pcm_val = 32767;

    if (pcm_val >= 256) {
        int temp = pcm_val >> 8;
        seg = 1;
        while (temp >= 2) {
            temp >>= 1;
            seg++;
        }
        pcm_val >>= (seg + 3);
        pcm_val &= 0x0F;
    } else {
        pcm_val >>= 4;
        pcm_val &= 0x0F;
        seg = 0;
    }
    return (uint8_t)((seg << 4) | pcm_val) ^ mask;
}

static int16_t alaw_to_linear(uint8_t a_val) {
    a_val ^= 0x55;
    int t = (a_val & 0x0F) << 4;
    int seg = (a_val & 0x70) >> 4;
    if (seg == 0) {
        t += 8;
    } else {
        t += 0x108;
        t <<= (seg - 1);
    }
    return (a_val & 0x80) ? -t : t;
}

#define BIAS_MULAW 0x84
static uint8_t linear_to_mulaw(int16_t pcm_val) {
    int mask = 0;
    int seg = 0;
    if (pcm_val < 0) {
        pcm_val = -pcm_val;
        mask = 0x7F;
    } else {
        mask = 0xFF;
    }
    if (pcm_val > 32635) pcm_val = 32635;
    pcm_val += BIAS_MULAW;

    int temp = pcm_val >> 7;
    while (temp >= 2) {
        temp >>= 1;
        seg++;
    }
    int quant = (pcm_val >> (seg + 3)) & 0x0F;
    return (uint8_t)((seg << 4) | quant) ^ mask;
}

static int16_t mulaw_to_linear(uint8_t u_val) {
    u_val ^= 0xFF;
    int t = ((u_val & 0x0F) << 3) + BIAS_MULAW + 4;
    int seg = (u_val & 0x70) >> 4;
    t <<= seg;
    t -= BIAS_MULAW;
    return (u_val & 0x80) ? -t : t;
}

int wav_compress_alaw(const WavAudio* src, WavAudio* dst) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }
    if (src->fmt.audio_format != 1u || src->fmt.bits_per_sample != 16u) {
        return -1;
    }

    uint32_t frame_count = src->data_size / src->fmt.block_align;
    uint32_t num_channels = src->fmt.num_channels;

    WavFormat out_fmt = src->fmt;
    out_fmt.audio_format = 6u; // A-law
    out_fmt.bits_per_sample = 8u;
    out_fmt.block_align = (uint16_t)num_channels;
    out_fmt.byte_rate = out_fmt.sample_rate * out_fmt.block_align;

    uint32_t out_data_size = frame_count * out_fmt.block_align;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    const uint8_t* in_data = src->bytes + src->data_offset;
    for (uint32_t i = 0; i < frame_count; i++) {
        const uint8_t* in_frame = in_data + i * src->fmt.block_align;
        uint8_t* out_frame = out_data + i * out_fmt.block_align;

        for (uint16_t c = 0; c < num_channels; c++) {
            int16_t sample = (int16_t)lire_u16_le(in_frame + c * 2u);
            out_frame[c] = linear_to_alaw(sample);
        }
    }

    int rc = wav_rebuild_pcm_bytes(dst, &out_fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

int wav_decompress_alaw(const WavAudio* src, WavAudio* dst) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }
    if (src->fmt.audio_format != 6u || src->fmt.bits_per_sample != 8u) {
        return -1;
    }

    uint32_t frame_count = src->data_size / src->fmt.block_align;
    uint32_t num_channels = src->fmt.num_channels;

    WavFormat out_fmt = src->fmt;
    out_fmt.audio_format = 1u; // PCM
    out_fmt.bits_per_sample = 16u;
    out_fmt.block_align = (uint16_t)(num_channels * 2u);
    out_fmt.byte_rate = out_fmt.sample_rate * out_fmt.block_align;

    uint32_t out_data_size = frame_count * out_fmt.block_align;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    const uint8_t* in_data = src->bytes + src->data_offset;
    for (uint32_t i = 0; i < frame_count; i++) {
        const uint8_t* in_frame = in_data + i * src->fmt.block_align;
        uint8_t* out_frame = out_data + i * out_fmt.block_align;

        for (uint16_t c = 0; c < num_channels; c++) {
            uint8_t alaw_val = in_frame[c];
            int16_t sample = alaw_to_linear(alaw_val);
            ecrire_u16_le(out_frame + c * 2u, (uint16_t)sample);
        }
    }

    int rc = wav_rebuild_pcm_bytes(dst, &out_fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

int wav_compress_mulaw(const WavAudio* src, WavAudio* dst) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }
    if (src->fmt.audio_format != 1u || src->fmt.bits_per_sample != 16u) {
        return -1;
    }

    uint32_t frame_count = src->data_size / src->fmt.block_align;
    uint32_t num_channels = src->fmt.num_channels;

    WavFormat out_fmt = src->fmt;
    out_fmt.audio_format = 7u; // mu-law
    out_fmt.bits_per_sample = 8u;
    out_fmt.block_align = (uint16_t)num_channels;
    out_fmt.byte_rate = out_fmt.sample_rate * out_fmt.block_align;

    uint32_t out_data_size = frame_count * out_fmt.block_align;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    const uint8_t* in_data = src->bytes + src->data_offset;
    for (uint32_t i = 0; i < frame_count; i++) {
        const uint8_t* in_frame = in_data + i * src->fmt.block_align;
        uint8_t* out_frame = out_data + i * out_fmt.block_align;

        for (uint16_t c = 0; c < num_channels; c++) {
            int16_t sample = (int16_t)lire_u16_le(in_frame + c * 2u);
            out_frame[c] = linear_to_mulaw(sample);
        }
    }

    int rc = wav_rebuild_pcm_bytes(dst, &out_fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

int wav_decompress_mulaw(const WavAudio* src, WavAudio* dst) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }
    if (src->fmt.audio_format != 7u || src->fmt.bits_per_sample != 8u) {
        return -1;
    }

    uint32_t frame_count = src->data_size / src->fmt.block_align;
    uint32_t num_channels = src->fmt.num_channels;

    WavFormat out_fmt = src->fmt;
    out_fmt.audio_format = 1u; // PCM
    out_fmt.bits_per_sample = 16u;
    out_fmt.block_align = (uint16_t)(num_channels * 2u);
    out_fmt.byte_rate = out_fmt.sample_rate * out_fmt.block_align;

    uint32_t out_data_size = frame_count * out_fmt.block_align;
    uint8_t* out_data = (uint8_t*)malloc(out_data_size);
    if (out_data == NULL) {
        return -1;
    }

    const uint8_t* in_data = src->bytes + src->data_offset;
    for (uint32_t i = 0; i < frame_count; i++) {
        const uint8_t* in_frame = in_data + i * src->fmt.block_align;
        uint8_t* out_frame = out_data + i * out_fmt.block_align;

        for (uint16_t c = 0; c < num_channels; c++) {
            uint8_t mulaw_val = in_frame[c];
            int16_t sample = mulaw_to_linear(mulaw_val);
            ecrire_u16_le(out_frame + c * 2u, (uint16_t)sample);
        }
    }

    int rc = wav_rebuild_pcm_bytes(dst, &out_fmt, out_data, out_data_size);
    free(out_data);
    return rc;
}

int wav_dynamic_range_compress(const WavAudio* src, WavAudio* dst, double threshold_db, double ratio, double makeup_gain_db) {
    if (src == NULL || dst == NULL || src->bytes == NULL) {
        return -1;
    }
    if (wav_clone(src, dst) != 0) {
        return -1;
    }
    if (dst->fmt.audio_format != 1u) {
        return -1;
    }

    uint16_t bps = dst->fmt.bits_per_sample;
    uint16_t sb = (uint16_t)(bps / 8u);
    if (sb == 0u) {
        return -1;
    }

    int32_t peak = max_amp(bps);
    if (peak <= 0) {
        return -1;
    }

    uint8_t* data = dst->bytes + dst->data_offset;
    uint32_t sample_count = dst->data_size / sb;

    double thresh_lin = pow(10.0, threshold_db / 20.0);
    double makeup_lin = pow(10.0, makeup_gain_db / 20.0);

    for (uint32_t i = 0; i < sample_count; i++) {
        uint8_t* p = data + i * sb;
        int32_t s = lire_echantillon(p, bps);
        double x = (double)s / (double)peak;
        double abs_x = fabs(x);

        if (abs_x > thresh_lin) {
            double compressed = thresh_lin + (abs_x - thresh_lin) / ratio;
            x = (x < 0.0) ? -compressed : compressed;
        }

        x *= makeup_lin;

        int32_t out = (int32_t)llround(x * (double)peak);
        int32_t clamped = borne_i32(out, -peak, peak);
        ecrire_echantillon(p, bps, clamped);
    }

    return 0;
}
