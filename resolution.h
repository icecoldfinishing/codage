#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <stdint.h>
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

#endif