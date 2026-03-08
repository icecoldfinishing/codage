#include <stdio.h>
#include <string.h>
#include "resolution.h"

// --- EXERCICE 0 : BASES ---

void conversion_octale_dynamique(uint16_t binaire) {
    // Tableau pour stocker les chiffres octaux (max 6 chiffres pour 16 bits)
    int chiffres[6]; 
    int i = 0;

    printf("Conversion dynamique de 0b");
    // Affichage binaire pour vérification
    for (int b = 15; b >= 0; b--) printf("%d", (binaire >> b) & 1);
    
    // BOUCLE DYNAMIQUE :
    // Tant qu'il reste des bits à traiter
    uint16_t temp = binaire;
    while (temp > 0) {
        // On isole les 3 bits de droite avec le masque 0x7 (111 en binaire)
        chiffres[i] = temp & 0x7;
        
        // On décale le nombre de 3 positions vers la droite pour passer au groupe suivant
        temp = temp >> 3;
        
        i++; // On passe à la case suivante du tableau
    }

    printf("\nResultat Octal: ");
    // On affiche le tableau à l'envers (car on a extrait de droite à gauche)
    if (i == 0) printf("0"); // Cas où le nombre vaut 0
    for (int j = i - 1; j >= 0; j--) {
        printf("%d", chiffres[j]);
    }
    printf("\n\n");
}

void afficher_permissions() {
    // Droits 755 [cite: 12]
    // 7=111, 5=101, 5=101
    printf("Exercice 0.1.2: Droits 755 en binaire: 111101101\n");

    // Droit 644 [cite: 13]
    // 6 = 110 (Lecture 4 + Ecriture 2), 4 = 100 (Lecture seule)
    printf("Exercice 0.1.3: Droit 644 signifie Lecture/Ecriture pour le proprio et Lecture seule pour les autres [cite: 11, 13]\n\n");
}

// --- EXERCICE 1 : DÉCODAGE VARIABLE ---

void analyser_octet_prefixe(uint8_t data) {
    printf("Exercice 1.1 & 1.2: Analyse de l'octet 0x%X\n", data); // 0xB2 = 10110010 [cite: 38]

    // Masquage pour extraire le premier bit (MSB) [cite: 41]
    // L'opération (data & 0x80) isole le bit le plus à gauche [cite: 42]
    uint8_t msb_result = data & 0x80;
    printf("Resultat de (data & 0x80): 0x%X (Bit a 1)\n", msb_result); // [cite: 43]

    // Décodage manuel (gauche à droite) selon le dictionnaire[cite: 36, 39]:
    // 10 -> A
    // 110 -> T
    // 0 -> E
    // 10 -> A
    printf("Sequence decodee: A, T, E, A\n");
    printf("Padding: Aucun bit reste inutilise [cite: 40]\n\n");
}

void encoder_message_huffman(const char* message) {
    printf("Exercice 1.3: Encodage de '%s'\n", message);
    uint8_t buffer = 0;
    int bits_in_buffer = 0;

    for (int i = 0; i < strlen(message); i++) {
        uint8_t code = 0;
        int len = 0;

        // Dictionnaire [cite: 36]
        switch(message[i]) {
            case 'E': code = 0; len = 1; break; // 0
            case 'A': code = 2; len = 2; break; // 10
            case 'T': code = 6; len = 3; break; // 110
            case 'N': code = 7; len = 3; break; // 111
        }

        // Ajout des bits dans l'octet via Shift (<<) et OR (|)
        for (int b = len - 1; b >= 0; b--) {
            uint8_t bit = (code >> b) & 1;
            buffer = (buffer << 1) | bit;
            bits_in_buffer++;

            if (bits_in_buffer == 8) {
                printf("Octet cree: 0x%02X\n", buffer);
                buffer = 0; bits_in_buffer = 0;
            }
        }
    }
    // Finalisation avec décalage si nécessaire (Padding)
    if (bits_in_buffer > 0) {
        buffer = buffer << (8 - bits_in_buffer);
        printf("Dernier octet (padding): 0x%02X\n", buffer);
    }
}