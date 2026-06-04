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

    printf("Etapes 1-2: Parsing + localisation chunk data\n");
    wav_print_info(&source);

    // --- COMPRESSION DU FICHIER ---
    printf("\n--- Compression du fichier WAV ---\n");
    bool has_decompressed = false;

    if (wav_compress_alaw(&source, &step_comp_alaw) == 0) {
        wav_write_file(&step_comp_alaw, "outputs/etape02_compressed_alaw.wav");
        printf("Compression A-law : Cree outputs/etape02_compressed_alaw.wav (Taille donnees: %u octets, gain de place: 50.0%%)\n", step_comp_alaw.data_size);

        if (wav_decompress_alaw(&step_comp_alaw, &step_decomp_alaw) == 0) {
            wav_write_file(&step_decomp_alaw, "outputs/etape02_decompressed_alaw.wav");
            printf("Decompression A-law : Cree outputs/etape02_decompressed_alaw.wav (Reconstruction PCM 16-bits)\n");
            has_decompressed = true;
        }
    } else {
        printf("Compression A-law ignoree (requiert une source 16-bits PCM).\n");
    }

    if (wav_compress_mulaw(&source, &step_comp_mulaw) == 0) {
        wav_write_file(&step_comp_mulaw, "outputs/etape02_compressed_mulaw.wav");
        printf("Compression mu-law : Cree outputs/etape02_compressed_mulaw.wav (Taille donnees: %u octets, gain de place: 50.0%%)\n", step_comp_mulaw.data_size);

        if (wav_decompress_mulaw(&step_comp_mulaw, &step_decomp_mulaw) == 0) {
            wav_write_file(&step_decomp_mulaw, "outputs/etape02_decompressed_mulaw.wav");
            printf("Decompression mu-law : Cree outputs/etape02_decompressed_mulaw.wav (Reconstruction PCM 16-bits)\n");
        }
    } else {
        printf("Compression mu-law ignoree (requiert une source 16-bits PCM).\n");
    }

    if (wav_dynamic_range_compress(&source, &step_drc, -12.0, 4.0, 3.0) == 0) {
        wav_write_file(&step_drc, "outputs/etape02_dynamic_compression.wav");
        printf("Compression de dynamique (DRC) : Cree outputs/etape02_dynamic_compression.wav (Seuil=-12dB, Ratio=4:1, Makeup=+3dB)\n");
    } else {
        printf("Compression de dynamique ignoree (requiert une source PCM).\n");
    }

    // Utilisation du son decompresse pour la suite du traitement
    WavAudio* working_source = &source;
    if (has_decompressed) {
        working_source = &step_decomp_alaw;
        printf("\n=> Les modifications suivantes seront appliquees sur le son decompresse (reconstruit depuis A-law).\n");
    } else {
        printf("\n=> Les modifications suivantes seront appliquees sur la source d'origine.\n");
    }

    // --- MODIFICATION DU SIGNAL ---
    printf("\n--- Modification et traitement du signal ---\n");

    if (wav_downsample_by_2(working_source, &step_downsample, false) == 0) {
        wav_write_file(&step_downsample, "outputs/etape03_downsample_x2.wav");
        printf("Etape 3 (Sous-echantillonnage) : Cree outputs/etape03_downsample_x2.wav\n");
    } else {
        printf("Etape 3 ignoree (format non supporte).\n");
    }

    if (wav_quantize_16_to_8(working_source, &step_quantize) == 0) {
        wav_write_file(&step_quantize, "outputs/etape04_quantization_8bit.wav");
        printf("Etape 4 (Quantification 8 bits) : Cree outputs/etape04_quantization_8bit.wav\n");
    } else {
        printf("Etape 4 ignoree : source non 16 bits PCM.\n");
    }

    if (wav_clone(working_source, &step_process) == 0) {
        WavStats sat;
        WavStats norm;
        wav_soft_desaturate_inplace(&step_process, &sat);
        wav_print_stats("Etape 5 (Saturation soft-clip)", &sat);
        wav_normalize_inplace(&step_process, 0.95, &norm);
        wav_print_stats("Etape 6 (Normalisation)", &norm);
        wav_write_file(&step_process, "outputs/etape05_06_dessat_norm.wav");
        printf("Etapes 5-6 : Cree outputs/etape05_06_dessat_norm.wav\n");
    }

    if (wav_extract_left_channel(working_source, &step_left) == 0) {
        wav_write_file(&step_left, "outputs/etape07_left_channel.wav");
        printf("Etape 7 (Extraction canal gauche) : Cree outputs/etape07_left_channel.wav\n");
    } else {
        printf("Etape 7 ignoree : source non stereo.\n");
    }

    // --- MULTICANAUX ---
    printf("\n--- Multicanaux ---\n");

    if (wav_stereo_to_2_1(working_source, &step_21, true) == 0) {
        wav_write_file(&step_21, "outputs/etape11_stereo_to_2_1.wav");
        printf("Etape 11 (Stereo vers 2.1) : Cree outputs/etape11_stereo_to_2_1.wav\n");
    } else {
        printf("Etape 11 ignoree : source non stereo.\n");
    }

    if (wav_stereo_to_5_1(working_source, &step_51) == 0) {
        wav_write_file(&step_51, "outputs/etape12_stereo_to_5_1.wav");
        printf("Etape 12 (Stereo vers 5.1) : Cree outputs/etape12_stereo_to_5_1.wav\n");
    } else {
        printf("Etape 12 ignoree : source non stereo.\n");
    }

    if (wav_generate_sine_5_1_travel(&step_synth, 48000, 16, 1.0, 440.0) == 0) {
        wav_write_file(&step_synth, "outputs/etape13_synth_5_1_travel.wav");
        printf("Etape 13 (Synthese 5.1 spatiale) : Cree outputs/etape13_synth_5_1_travel.wav\n");
    }

    printf("\nEtape 10 (test auditif) : lecture du fichier de synthese...\n");
    if (wav_play_file_simple("outputs/etape13_synth_5_1_travel.wav") != 0) {
        printf("Lecture non disponible automatiquement sur cet environnement.\n");
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