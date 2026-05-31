#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "wav_dsp.h"

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
