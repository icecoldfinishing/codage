#ifndef BASES_H
#define BASES_H

#include <stdint.h>

typedef struct {
    char lettre;
    uint8_t code;
    uint8_t longueur;
} ReglePrefixe;

void conversion_base(uint16_t valeur, uint8_t base);
void decodage(uint8_t data);
void encodage(const char* message);

#endif
