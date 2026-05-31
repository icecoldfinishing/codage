#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include "simd_utils.h"

void simd(const uint8_t* A, const uint8_t* B, uint8_t* Res, int n) {
    for (int i = 0; i <= n - 32; i += 32) {
        __m256i vecA = _mm256_loadu_si256((const __m256i*)&A[i]);
        __m256i vecB = _mm256_loadu_si256((const __m256i*)&B[i]);
        __m256i masque_resultat = _mm256_cmpgt_epi8(vecA, vecB);
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
