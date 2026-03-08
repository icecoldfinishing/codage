#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <stdint.h>
#include <immintrin.h>

// Définition de la structure pour le dictionnaire dynamique
typedef struct {
    char lettre;
    uint8_t code;
    uint8_t longueur;
} ReglePrefixe;

// --- Exercice 0 : Bases ---
void conversion_octale_dynamique(uint16_t binaire);

// --- Exercice 1 : Codage Variable ---
void analyser_octet_prefixe(uint8_t data);
void encoder_message_huffman(const char* message);

// --- Exercice 2 : Parallélisme SIMD ---
void resoudre_exercice_2_simd(const uint8_t* A, const uint8_t* B, uint8_t* Res, int n);

// --- Exercice 3 : Recherche Rapide (Movemask) ---
int localiser_premier_zero_simd(const uint8_t* ptr);
void rechercher_caractere_simd(const uint8_t* ptr, uint8_t cible);

#endif