#include <stdio.h>
#include <stdint.h>
#include "resolution.h"

int main() {
    // --- Initialisation standard ---
    printf("--- TEST DE LANCEMENT ---\n");
    fflush(stdout); 

    // --- Exercice 0 : Conversion Octale ---
    // Convertit 110 101 001 (octal: 651)
    conversion_octale(0b110101001); 

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

    // --- Exercice 3 : Recherche Movemask SSE ---
    // Chaîne avec un caractère nul à l'index 5
    uint8_t chaine[16] = "Hello\0World!!"; 
    int index_zero = localiser_premier_zero_simd(chaine);
    printf("Exercice 3.2 : Premier '\\0' trouve a l'index : %d\n", index_zero);

    // Recherche d'un caractère arbitraire 'W' (0x57)
    rechercher_caractere_simd(chaine, 'W');

    return 0;
}