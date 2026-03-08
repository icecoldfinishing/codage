#include <stdio.h>
#include "resolution.h"

int main() {
    printf("=== Lancement du TD 1 ===\n\n");

    resoudre();
    
    printf("\n");
    
    // Exercice 1 : Octet 0xB2 (10110010) [cite: 38]
    decoder_octet(0xB2);

    return 0;
}