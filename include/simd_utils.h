#ifndef SIMD_UTILS_H
#define SIMD_UTILS_H

#include <stdint.h>

void simd(const uint8_t* A, const uint8_t* B, uint8_t* Res, int n);
int localiser_premier_zero_simd(const uint8_t* ptr);
int rechercher_caractere_simd(const uint8_t* ptr, uint8_t cible);

#endif
