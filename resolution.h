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
void conversion_base(uint16_t valeur, uint8_t base);

// --- Exercice 1 : Codage Variable ---
void decodage(uint8_t data);
void encodage(const char* message);

// --- Exercice 2 : Parallélisme SIMD ---
void simd(const uint8_t* A, const uint8_t* B, uint8_t* Res, int n);

// --- Exercice 3 : Recherche Rapide (Movemask) ---
int localiser_premier_zero_simd(const uint8_t* ptr);
int rechercher_caractere_simd(const uint8_t* ptr, uint8_t cible);

#endif