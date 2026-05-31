#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "conversions.h"

static uint16_t masque_nb_bits(uint8_t nb_bits) {
    if (nb_bits >= 16) {
        return 0xFFFFu;
    }
    return (uint16_t)((1u << nb_bits) - 1u);
}

void convertisseur_init(ConvertisseurNumerique* self, uint8_t nb_bits_signed, double epsilon) {
    if (self == NULL) {
        return;
    }
    self->nb_bits_signed = (nb_bits_signed < 2 || nb_bits_signed > 16) ? 8 : nb_bits_signed;
    self->epsilon = (epsilon > 0.0) ? epsilon : 1e-6;
    self->buffer[0] = '\0';
}

uint16_t convertir_vers_signed_msb(const ConvertisseurNumerique* self, int16_t valeur) {
    if (self == NULL) {
        return 0;
    }

    uint8_t n = self->nb_bits_signed;
    uint8_t magnitude_bits = (uint8_t)(n - 1);
    uint16_t signe = (valeur < 0) ? 1u : 0u;

    int32_t abs_val = (valeur < 0) ? -(int32_t)valeur : (int32_t)valeur;
    uint16_t magnitude_max = (uint16_t)((1u << magnitude_bits) - 1u);
    if (abs_val > magnitude_max) {
        abs_val = magnitude_max;
    }

    uint16_t code = (uint16_t)((signe << magnitude_bits) | (uint16_t)abs_val);
    return (uint16_t)(code & masque_nb_bits(n));
}

int16_t convertir_depuis_signed_msb(const ConvertisseurNumerique* self, uint16_t code) {
    if (self == NULL) {
        return 0;
    }

    uint8_t n = self->nb_bits_signed;
    uint8_t magnitude_bits = (uint8_t)(n - 1);
    uint16_t masque = masque_nb_bits(n);
    uint16_t code_normalise = (uint16_t)(code & masque);

    uint16_t signe = (uint16_t)((code_normalise >> magnitude_bits) & 0x1u);
    uint16_t magnitude = (uint16_t)(code_normalise & ((1u << magnitude_bits) - 1u));

    return (signe == 1u) ? (int16_t)(-(int16_t)magnitude) : (int16_t)magnitude;
}

uint32_t encoder_ieee754_simple_precision(float valeur) {
    union {
        float f;
        uint32_t u;
    } conv;
    conv.f = valeur;
    return conv.u;
}

IEEE754Simple decomposer_ieee754_simple_precision(float valeur) {
    IEEE754Simple info;
    info.brut = encoder_ieee754_simple_precision(valeur);
    info.signe = (uint8_t)((info.brut >> 31) & 0x1u);
    info.exposant_biase = (uint8_t)((info.brut >> 23) & 0xFFu);
    info.mantisse = info.brut & 0x7FFFFFu;
    return info;
}

const char* ieee754_simple_precision_vers_binaire_1_8_23(float valeur, char* sortie, uint32_t taille_sortie) {
    if (sortie == NULL || taille_sortie < 36) {
        return NULL;
    }

    uint32_t brut = encoder_ieee754_simple_precision(valeur);
    uint32_t pos = 0;

    sortie[pos++] = (char)('0' + ((brut >> 31) & 0x1u));
    sortie[pos++] = '|';

    for (int i = 30; i >= 23; i--) {
        sortie[pos++] = (char)('0' + ((brut >> i) & 0x1u));
    }
    sortie[pos++] = '|';

    for (int i = 22; i >= 0; i--) {
        sortie[pos++] = (char)('0' + ((brut >> i) & 0x1u));
    }

    sortie[pos] = '\0';
    return sortie;
}

const char* entier_relatif_vers_base(const ConvertisseurNumerique* self, int32_t valeur, uint8_t base, char* sortie, uint32_t taille_sortie) {
    if (self == NULL || sortie == NULL || taille_sortie < 3) {
        return NULL;
    }
    if (base < 2 || base > 16) {
        snprintf(sortie, taille_sortie, "Base invalide");
        return sortie;
    }

    const char* alphabet = "0123456789ABCDEF";
    uint32_t pos = 0;
    uint32_t abs_val;

    if (valeur < 0) {
        sortie[pos++] = '-';
        abs_val = (uint32_t)(-(int64_t)valeur);
    } else {
        abs_val = (uint32_t)valeur;
    }

    char inverse[64];
    uint32_t len = 0;
    if (abs_val == 0u) {
        inverse[len++] = '0';
    } else {
        while (abs_val > 0u && len < (uint32_t)(sizeof(inverse) - 1u)) {
            inverse[len++] = alphabet[abs_val % base];
            abs_val /= base;
        }
    }

    for (int32_t i = (int32_t)len - 1; i >= 0; i--) {
        if (pos + 1 >= taille_sortie) {
            sortie[taille_sortie - 1] = '\0';
            return sortie;
        }
        sortie[pos++] = inverse[i];
    }

    sortie[pos] = '\0';
    return sortie;
}

const char* decimal_vers_binaire(const ConvertisseurNumerique* self, double valeur, char* sortie, uint32_t taille_sortie) {
    if (self == NULL || sortie == NULL || taille_sortie < 4) {
        return NULL;
    }

    if (isnan(valeur)) {
        snprintf(sortie, taille_sortie, "NaN");
        return sortie;
    }
    if (isinf(valeur)) {
        snprintf(sortie, taille_sortie, (valeur < 0) ? "-Inf" : "Inf");
        return sortie;
    }

    double x = valeur;
    uint32_t pos = 0;

    if (x < 0.0) {
        sortie[pos++] = '-';
        x = -x;
    }

    unsigned long long partie_entiere = (unsigned long long)floor(x);
    double partie_fractionnaire = x - (double)partie_entiere;

    char entier_inverse[96];
    uint32_t len_entier = 0;
    if (partie_entiere == 0ull) {
        entier_inverse[len_entier++] = '0';
    } else {
        while (partie_entiere > 0ull && len_entier < (uint32_t)(sizeof(entier_inverse) - 1u)) {
            entier_inverse[len_entier++] = (char)('0' + (partie_entiere & 1ull));
            partie_entiere >>= 1;
        }
    }

    for (int32_t i = (int32_t)len_entier - 1; i >= 0; i--) {
        if (pos + 1 >= taille_sortie) {
            sortie[taille_sortie - 1] = '\0';
            return sortie;
        }
        sortie[pos++] = entier_inverse[i];
    }

    if (partie_fractionnaire <= self->epsilon) {
        sortie[pos] = '\0';
        return sortie;
    }

    if (pos + 2 >= taille_sortie) {
        sortie[taille_sortie - 1] = '\0';
        return sortie;
    }

    sortie[pos++] = '.';

    uint32_t garde = 0;
    while (partie_fractionnaire > self->epsilon && garde < (taille_sortie - pos - 1u)) {
        partie_fractionnaire *= 2.0;
        if (partie_fractionnaire >= 1.0) {
            sortie[pos++] = '1';
            partie_fractionnaire -= 1.0;
        } else {
            sortie[pos++] = '0';
        }
        garde++;
    }

    sortie[pos] = '\0';
    return sortie;
}
