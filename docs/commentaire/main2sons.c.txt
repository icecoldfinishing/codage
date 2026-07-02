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
    const char* input_wav = (argc > 1 && argv[1] != NULL && strlen(argv[1]) > 0) ? argv[1] : "input.wav";
    if (run_wav_tp(input_wav) != 0) {
        printf("Le pipeline WAV a rencontre une erreur.\n");
    }

    return 0;
}