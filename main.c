#include <stdio.h>
#include <stdint.h>
#include "resolution.h"

int main() {
    // --- Initialisation standard ---
    printf("--- TEST DE LANCEMENT ---\n");
    fflush(stdout); 

    // --- Exercice 0 : Conversion en base dynamique ---
    // Convertit 110 101 001 en base 8 (resultat attendu: 651)
    conversion_base(0b110101001, 8); 

    // --- Exercice 1 : Décodage et Encodage ---
    // Décodage de 0xB2 (10110010)
    decodage(0xB2);
    // Encodage selon le dictionnaire
    encodage("ATEA");

    printf("--- TESTS SIMD (EXERCICES 2 & 3) ---\n");

    // --- Exercice 2 : Comparaison de seuil AVX2 ---
    // On prépare 32 octets pour remplir un registre 256 bits
    uint8_t tableau_A[32], tableau_B[32], resultat[32];
    for(int i = 0; i < 32; i++) {
        tableau_A[i] = i * 2;  // 0, 2, 4, ...
        tableau_B[i] = 30;     // Seuil fixe à 30
    }

    simd(tableau_A, tableau_B, resultat, 32);

    printf("Resultat SIMD: ");
    for(int i = 0; i < 32; i++) {
        // A[16] est le premier > 30 (16*2=32)
        if (i % 8 == 0) printf("\n  "); 
        printf("%02X ", resultat[i]);
    }
    printf("\n\n");

    // Chaîne avec un caractère nul à l'index 5
    uint8_t chaine[16] = "Hello\0World!!"; 
    int index_zero = localiser_premier_zero_simd(chaine);
    printf("Exercice 3.2 : Premier '\\0' trouve a l'index : %d\n", index_zero);

    // Recherche d'un caractère arbitraire 'W' (0x57)
    int index_caractere = rechercher_caractere_simd(chaine, 'W');
    printf("Caractere '%c' trouve a l'index : %d\n", 'W', index_caractere);

    printf("\n--- EXERCICE 4 : CONVERSIONS OO ---\n");

    ConvertisseurNumerique conv;
    convertisseur_init(&conv, 8, 1e-6);

    int16_t valeur_signee = -37;
    uint16_t code_msb = convertir_vers_signed_msb(&conv, valeur_signee);
    int16_t retour_signee = convertir_depuis_signed_msb(&conv, code_msb);
    printf("Signed-MSB (%u bits): %d -> 0x%02X -> %d\n", conv.nb_bits_signed, valeur_signee, code_msb, retour_signee);

    if (entier_relatif_vers_base(&conv, valeur_signee, 2, conv.buffer, BINAIRE_BUFFER_TAILLE) != NULL) {
        printf("Entier relatif dynamique base 2 : %d -> %s\n", valeur_signee, conv.buffer);
    }
    if (entier_relatif_vers_base(&conv, valeur_signee, 8, conv.buffer, BINAIRE_BUFFER_TAILLE) != NULL) {
        printf("Entier relatif dynamique base 8 : %d -> %s\n", valeur_signee, conv.buffer);
    }
    if (entier_relatif_vers_base(&conv, valeur_signee, 16, conv.buffer, BINAIRE_BUFFER_TAILLE) != NULL) {
        printf("Entier relatif dynamique base 16 : %d -> %s\n", valeur_signee, conv.buffer);
    }

    float valeur_float = -13.625f;
    IEEE754Simple ieee = decomposer_ieee754_simple_precision(valeur_float);
    char ieee_bits_1_8_23[40];

    printf("IEEE754 simple precision de %.3f\n", valeur_float);
    if (ieee754_simple_precision_vers_binaire_1_8_23(valeur_float, ieee_bits_1_8_23, sizeof(ieee_bits_1_8_23)) != NULL) {
        printf("  Base 2 (32 bits) : %s\n", ieee_bits_1_8_23);
        printf("  Format           : signe(1) | exposant(8) | mantisse(23)\n");
    }
    printf("  Signe    : %u\n", ieee.signe);
    printf("  Exposant : 0x%02X (%u)\n", ieee.exposant_biase, ieee.exposant_biase);
    printf("  Mantisse : 0x%06X\n", ieee.mantisse);
    printf("  Brut     : 0x%08X\n", ieee.brut);

    if (decimal_vers_binaire(&conv, 10.625, conv.buffer, BINAIRE_BUFFER_TAILLE) != NULL) {
        printf("Decimal vers binaire (eps=%g) : 10.625 -> %s\n", conv.epsilon, conv.buffer);
    }


    return 0;
}