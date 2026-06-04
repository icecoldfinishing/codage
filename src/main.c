#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "resolution.h"

static int run_wav_tp(const char* input_path) {
    WavAudio source;
    WavAudio step_comp_alaw;
    WavAudio step_decomp_alaw;
    WavAudio step_comp_mulaw;
    WavAudio step_decomp_mulaw;
    WavAudio step_drc;
    WavAudio step_downsample;
    WavAudio step_quantize;
    WavAudio step_process;
    WavAudio step_left;
    WavAudio step_21;
    WavAudio step_51;
    WavAudio step_synth;

    wav_init(&source);
    wav_init(&step_comp_alaw);
    wav_init(&step_decomp_alaw);
    wav_init(&step_comp_mulaw);
    wav_init(&step_decomp_mulaw);
    wav_init(&step_drc);
    wav_init(&step_downsample);
    wav_init(&step_quantize);
    wav_init(&step_process);
    wav_init(&step_left);
    wav_init(&step_21);
    wav_init(&step_51);
    wav_init(&step_synth);

    printf("\n--- TP WAV : Manipulation Binaire ---\n");

    if (wav_load_file(&source, input_path) != 0 || wav_parse_header(&source) != 0) {
        printf("Source '%s' absente ou invalide. Generation d'une source stereo de secours...\n", input_path);
        if (wav_generate_sine_stereo(&source, 44100, 16, 2.0, 440.0) != 0) {
            printf("Echec: impossible de generer une source de secours.\n");
            goto cleanup_fail;
        }
        if (wav_write_file(&source, "outputs/etape00_source_synthetique.wav") == 0) {
            printf("Cree: outputs/etape00_source_synthetique.wav\n");
        }
    }

    if (wav_parse_header(&source) != 0) {
        printf("Echec du parsing RIFF/WAV.\n");
        goto cleanup_fail;
    }

    printf("\n--- Etape 1 : Analyse de la structure (Parsing) ---\n");
    printf("1. Lecture de l'en-tete / 2. Identification des donnees\n");
    wav_print_info(&source);

    // --- MODIFICATION DU SIGNAL ---
    printf("\n--- Etape 2 : Reduction de la qualite (Sous-echantillonnage) ---\n");

    if (wav_downsample_by_2(&source, &step_downsample, false) == 0) {
        wav_write_file(&step_downsample, "outputs/etape02_3_downsample.wav");
        printf("3. Changement d'echantillonnage : Cree outputs/etape02_3_downsample.wav\n");
    } else {
        printf("3. Changement d'echantillonnage ignore (format non supporte).\n");
    }

    if (wav_quantize_16_to_8(&source, &step_quantize) == 0) {
        wav_write_file(&step_quantize, "outputs/etape02_4_quantization_8bit.wav");
        printf("4. Changement de quantification : Cree outputs/etape02_4_quantization_8bit.wav\n");
    } else {
        printf("4. Changement de quantification ignore : source non 16 bits PCM.\n");
    }

    printf("\n--- Etape 3 : Traitement du Signal (Arithmetique) ---\n");
    if (wav_clone(&source, &step_process) == 0) {
        WavStats sat;
        WavStats norm;
        wav_soft_desaturate_inplace(&step_process, &sat);
        wav_print_stats("5. Gestion de la saturation", &sat);
        wav_normalize_inplace(&step_process, 0.95, &norm);
        wav_print_stats("6. Normalisation", &norm);
        wav_write_file(&step_process, "outputs/etape03_5_6_process.wav");
        printf("Cree outputs/etape03_5_6_process.wav\n");
    }

    printf("\n--- Etape 4 : Spatialisation et Synthese ---\n");
    if (wav_extract_left_channel(&source, &step_left) == 0) {
        wav_write_file(&step_left, "outputs/etape04_7_left_channel.wav");
        printf("7. Extraction de canaux : Cree outputs/etape04_7_left_channel.wav\n");
    } else {
        printf("7. Extraction de canaux ignoree : source non stereo.\n");
    }

    printf("\n--- Etape 5 : Exportation et Validation ---\n");
    printf("9. Sauvegarde du fichier... les fichiers ont ete sauvegardes via nos fonctions.\n");
    printf("10. Test auditif : lecture du fichier de source modifie...\n");
    if (wav_play_file_simple("outputs/etape03_5_6_process.wav") != 0) {
        printf("Lecture non disponible automatiquement sur cet environnement.\n");
    }

    printf("\n--- Etape 6 : Spatialisation et Synthese (Extension Multicanaux) ---\n");

    if (wav_stereo_to_2_1(&source, &step_21, true) == 0) {
        wav_write_file(&step_21, "outputs/etape06_11_stereo_to_2_1.wav");
        printf("11. Passage de la Stereo (2.0) au 2.1 : Cree outputs/etape06_11_stereo_to_2_1.wav\n");
    } else {
        printf("11. Passage au 2.1 ignore : source non stereo.\n");
    }

    if (wav_stereo_to_5_1(&source, &step_51) == 0) {
        wav_write_file(&step_51, "outputs/etape06_12_stereo_to_5_1.wav");
        printf("12. Simulation de Surround 5.1 (Up-mixing) : Cree outputs/etape06_12_stereo_to_5_1.wav\n");
    } else {
        printf("12. Simulation Surround ignoree : source non stereo.\n");
    }

    if (wav_generate_sine_5_1_travel(&step_synth, 48000, 16, 1.0, 440.0) == 0) {
        wav_write_file(&step_synth, "outputs/etape06_13_synth_5_1_travel.wav");
        printf("13. Generation d'ondes (Le synthetiseur) : Cree outputs/etape06_13_synth_5_1_travel.wav\n");
    }

    wav_free(&source);
    wav_free(&step_comp_alaw);
    wav_free(&step_decomp_alaw);
    wav_free(&step_comp_mulaw);
    wav_free(&step_decomp_mulaw);
    wav_free(&step_drc);
    wav_free(&step_downsample);
    wav_free(&step_quantize);
    wav_free(&step_process);
    wav_free(&step_left);
    wav_free(&step_21);
    wav_free(&step_51);
    wav_free(&step_synth);
    return 0;

cleanup_fail:
    wav_free(&source);
    wav_free(&step_comp_alaw);
    wav_free(&step_decomp_alaw);
    wav_free(&step_comp_mulaw);
    wav_free(&step_decomp_mulaw);
    wav_free(&step_drc);
    wav_free(&step_downsample);
    wav_free(&step_quantize);
    wav_free(&step_process);
    wav_free(&step_left);
    wav_free(&step_21);
    wav_free(&step_51);
    wav_free(&step_synth);
    return -1;
}

int main(int argc, char** argv) {
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

    const char* input_wav = (argc > 1 && argv[1] != NULL && strlen(argv[1]) > 0) ? argv[1] : "input.wav";
    if (run_wav_tp(input_wav) != 0) {
        printf("Le pipeline WAV a rencontre une erreur.\n");
    }

    return 0;
}