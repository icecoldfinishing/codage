#include <stdio.h>
#include "resolution.h"

int main() {
    // Ce printf DOIT s'afficher quoi qu'il arrive
    printf("--- TEST DE LANCEMENT ---\n");
    fflush(stdout); 

    conversion_octale_dynamique(0b110101001);
    afficher_permissions();
    analyser_octet_prefixe(0xB2);
    encoder_message_huffman("ATEA");

    return 0;
}