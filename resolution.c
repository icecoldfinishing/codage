#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "resolution.h"

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
    // Parcourt les bits de l'octet (de gauche à droite)
    for (int i = 7; i >= 0; i--) {
        // Extrait le bit i et l'ajoute au registre
        uint8_t bit = (data >> i) & 1;
        registre = (registre << 1) | bit;
        len++;
        // Recherche une correspondance dans le dictionnaire
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
    // On parcourt chaque lettre du message à encoder
    for (int i = 0; i < strlen(message); i++) {
        char lettre_cible = message[i];
        bool trouve = false;
        // On cherche la lettre dans le dictionnaire
        for (int r = 0; r < nb_regles; r++) {
            if (dictionnaire[r].lettre == lettre_cible) {
                trouve = true;
                uint8_t code = dictionnaire[r].code;
                int len = dictionnaire[r].longueur;
                // On insère les bits du code un par un dans le buffer
                for (int b = len - 1; b >= 0; b--) {
                    uint8_t bit = (code >> b) & 1; // Extraction du bit
                    buffer = (buffer << 1) | bit;  // Insertion dans le buffer
                    bits_utilises++;
                    // Si le buffer est plein (8 bits), on l'affiche et on reset
                    if (bits_utilises == 8) {
                        printf("Octet genere : 0x%02X\n", buffer);
                        buffer = 0;
                        bits_utilises = 0;
                    }
                }
                break; // Lettre trouvée et traitée, on passe à la suivante
            }
        }
        if (!trouve) printf("(Lettre '%c' inconnue du dictionnaire)\n", lettre_cible);
    }
    // Gestion du Padding : si le dernier octet n'est pas complet
    if (bits_utilises > 0) {
        // On décale les bits vers la gauche pour remplir l'octet (MSB align)
        uint8_t final_octet = buffer << (8 - bits_utilises);
        printf("Dernier octet (avec padding 0) : 0x%02X\n", final_octet);
    }
    printf("\n");
}

/**
 * @brief Compare deux tableaux via SIMD AVX2.
 * @details Remplace 32 itérations classiques par 1 seule comparaison vectorielle.
 * Res[i] = 0xFF si A[i] > B[i], sinon 0x00. [cite: 52, 53]
 */
void simd(const uint8_t* A, const uint8_t* B, uint8_t* Res, int n) {
    // On traite les données par blocs de 32 octets (256 bits) [cite: 48, 55]
    for (int i = 0; i <= n - 32; i += 32) {
        
        // 1. Chargement des données (LOD)
        // On utilise _mm256_loadu_si256 pour éviter un crash si l'adresse n'est pas alignée sur 32 octets [cite: 59, 60]
        __m256i vecA = _mm256_loadu_si256((const __m256i*)&A[i]);
        __m256i vecB = _mm256_loadu_si256((const __m256i*)&B[i]);

        // 2. Comparaison Vectorielle (CMP)
        // L'instruction _mm256_cmpgt_epi8 effectue 32 comparaisons 'A[i] > B[i]' en parallèle 
        __m256i masque_resultat = _mm256_cmpgt_epi8(vecA, vecB);

        // 3. Stockage (STR)
        _mm256_storeu_si256((__m256i*)&Res[i], masque_resultat);
    }
}

int localiser_premier_zero_simd(const uint8_t* ptr) {
    __m128i target = _mm_set1_epi8(0x00); 
    __m128i data = _mm_loadu_si128((const __m128i*)ptr); 
    __m128i cmp = _mm_cmpeq_epi8(data, target);
    int mask = _mm_movemask_epi8(cmp);
    if (mask == 0) return -1; 
    return __builtin_ctz(mask); 
}

int rechercher_caractere_simd(const uint8_t* ptr, uint8_t cible) {
    __m128i target = _mm_set1_epi8(cible); 
    __m128i data = _mm_loadu_si128((const __m128i*)ptr);
    __m128i cmp = _mm_cmpeq_epi8(data, target); 
    int mask = _mm_movemask_epi8(cmp);
    printf("Masque de recherche pour '%c' (0x%02X) : 0x%04X\n", cible, cible, mask);
    if (mask == 0) return -1;
    return __builtin_ctz(mask); 
}

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