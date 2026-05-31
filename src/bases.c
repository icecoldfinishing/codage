#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "bases.h"

ReglePrefixe dictionnaire[] = {
    {'E', 0, 1},   
    {'A', 2, 2},   
    {'T', 6, 3},   
    {'N', 7, 3}    
};
int nb_regles = 4;

void conversion_base(uint16_t valeur, uint8_t base) {
    char chiffres[16];
    int i = 0;
    const char* alphabet = "0123456789ABCDEF";

    if (base < 2 || base > 16) {
        printf("Base invalide (%u). Utilisez une base entre 2 et 16.\n\n", base);
        return;
    }

    printf("Conversion dynamique de 0b");
    for (int b = 15; b >= 0; b--) printf("%d", (valeur >> b) & 1);

    uint16_t temp = valeur;
    while (temp > 0) {
        chiffres[i] = alphabet[temp % base];
        temp = temp / base;
        i++;
    }

    printf("\nResultat base %u: ", base);
    if (i == 0) printf("0");
    for (int j = i - 1; j >= 0; j--) {
        printf("%c", chiffres[j]);
    }
    printf("\n\n");
}

void decodage(uint8_t data) {
    printf("Exercice 1 : Decodage de 0x%02X\n", data);
    uint8_t registre = 0; 
    int len = 0;         
    printf("Sequence : ");
    for (int i = 7; i >= 0; i--) {
        uint8_t bit = (data >> i) & 1;
        registre = (registre << 1) | bit;
        len++;
        for (int r = 0; r < nb_regles; r++) {
            if (len == dictionnaire[r].longueur && registre == dictionnaire[r].code) {
                printf("%c ", dictionnaire[r].lettre);
                registre = 0;
                len = 0;
                break;
            }
        }
    }
    printf("\n\n");
}

void encodage(const char* message) {
    printf("Exercice 1.3: Encodage dynamique de '%s'\n", message);
    uint8_t buffer = 0;
    int bits_utilises = 0;
    for (size_t i = 0; i < strlen(message); i++) {
        char lettre_cible = message[i];
        bool trouve = false;
        for (int r = 0; r < nb_regles; r++) {
            if (dictionnaire[r].lettre == lettre_cible) {
                trouve = true;
                uint8_t code = dictionnaire[r].code;
                int len = dictionnaire[r].longueur;
                for (int b = len - 1; b >= 0; b--) {
                    uint8_t bit = (code >> b) & 1;
                    buffer = (buffer << 1) | bit;
                    bits_utilises++;
                    if (bits_utilises == 8) {
                        printf("Octet genere : 0x%02X\n", buffer);
                        buffer = 0;
                        bits_utilises = 0;
                    }
                }
                break;
            }
        }
        if (!trouve) printf("(Lettre '%c' inconnue du dictionnaire)\n", lettre_cible);
    }
    if (bits_utilises > 0) {
        uint8_t final_octet = buffer << (8 - bits_utilises);
        printf("Dernier octet (avec padding 0) : 0x%02X\n", final_octet);
    }
    printf("\n");
}
