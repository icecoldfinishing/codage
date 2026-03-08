#include <stdio.h>
#include "resolution.h"

void resoudre() {
    // 1.1 Conversion binaire 110 101 001 en octal
    // En binaire: 110=6, 101=5, 001=1 -> Octal: 651 [cite: 9, 10]
    printf("Exercice 0.1: 110 101 001 binaire = 0651 octal\n");

    // 1.2 Permissions 755 [cite: 11, 12]
    // 7 (111), 5 (101), 5 (101)
    uint16_t p755 = 0b111101101; 
    printf("Permissions 755 en binaire: 111 101 101\n");
}

void decoder_octet(uint8_t data) {
    printf("Décodage de l'octet 0x%X :\n", data);
    
    // On utilise un masque pour isoler le bit de poids fort (MSB) 
    // 0x80 vaut 10000000 en binaire 
    uint8_t premier_bit = (data & 0x80) >> 7; 
    
    if (premier_bit == 1) {
        printf("Le premier bit est 1 (Résultat de l'opération & 0x80 : 0x80)\n"); 
    }else {
        printf("Le premier bit est 0\n");
    }
    
    // Selon le dictionnaire[cite: 36, 39]: 
    // 10110010 se décompose en 10 (A), 110 (T), 0 (E), 10 (A)
    printf("Séquence : A (10), T (110), E (0), A (10)\n");
}