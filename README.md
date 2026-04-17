# codage

# Projet : Traitement de Données Haute Performance (SIMD & Bitwise)

## Présentation
Ce projet implémente des algorithmes de traitement de données optimisés en langage C. L'objectif est de maximiser l'utilisation des ressources matérielles en exploitant les capacités des processeurs modernes via l'architecture x86_64.

Le code met en œuvre trois piliers fondamentaux de l'informatique de bas niveau :
1. **Manipulation de flux de bits** : Décodage efficace et conversion de bases numériques.
2. **Parallélisme de données (SIMD)** : Utilisation des registres SSE et AVX2 pour traiter plusieurs octets en un seul cycle d'horloge.
3. **Gestion de la mémoire** : Optimisation des accès via le contrôle de l'alignement des données et l'utilisation de fenêtres glissantes.

## Concepts Techniques

### Vectorisation (SIMD)
Au lieu de traiter les données octet par octet, le projet utilise le mode Single Instruction, Multiple Data (SIMD).
* **SSE (128 bits)** : Traitement de 16 octets simultanément.
* **AVX2 (256 bits)** : Traitement de 32 octets simultanément.

### Algorithmes de Masquage
Le projet utilise des techniques de masquage binaire pour :
* Isoler des informations spécifiques via des masques ET (AND).
* Compacter les résultats de comparaisons complexes en masques de bits simples (Movemask).
* Localiser des motifs dans des flux de données sans utiliser de branchements conditionnels (if), évitant ainsi les pénalités de prédiction de branchement.

### Optimisation des Accès Mémoire
Le projet gère les contraintes d'alignement mémoire. L'utilisation stratégique des chargements non-alignés (Unaligned Load) permet de traiter des données contiguës et des décalages d'octets indispensables pour les vérifications d'ordre ou les recherches de sous-séquences.

## Environnement Technique
Le projet nécessite un compilateur supportant les intrinsèques Intel/AMD :
* **Compilateur** : GCC ou Clang.
* **Drapeaux de compilation** : `-mavx2` et `-msse4.1`.

## Performance et Objectifs
* Réduction significative du nombre d'instructions par rapport aux boucles scalaires.
* Augmentation du débit de données pour les opérations de filtrage et de recherche.
* Optimisation de l'empreinte CPU par l'exploitation des unités de calcul vectorielles.

## TP WAV (Binaire pur, sans lib de parsing)

Le projet contient maintenant un pipeline complet orienté objet en C (struct + fonctions) pour manipuler un fichier WAV en mode binaire Little Endian.

### Lancer

Compilation (Linux/WSL/MSYS2):

```bash
gcc -O2 -mavx2 -msse4.1 -o codage main.c resolution.c -lm
```

Exécution avec un WAV d'entrée:

```bash
./codage input.wav
```

Si `input.wav` est absent, le programme génère automatiquement une source stéréo de secours.

### Étapes implémentées

1. Parsing en-tête RIFF/WAVE (`fmt`, `data`) et extraction:
	* fréquence, canaux, quantification, `ByteRate`, `BlockAlign`
	* offset exact des données audio
2. Sous-échantillonnage ×2:
	* fusion de 2 frames (moyenne ou max)
	* mise à jour en-tête: `SampleRate`, `ByteRate`, tailles
3. Quantification 16→8 bits:
	* conversion d'amplitude en conservant la forme globale
4. Dé-saturation:
	* soft-clip (`tanh`) pour éviter l'écrêtage brutal
5. Normalisation:
	* recherche du pic et gain global sans dépassement
6. Extraction canal gauche (stéréo entrelacé)
7. Export WAV:
	* reconstruction d'un en-tête PCM valide
8. Test auditif simple:
	* Windows: `Media.SoundPlayer`
	* Linux: `ffplay`
9. Extension 2.1:
	* création canal LFE = moyenne(L,R)
	* filtre passe-bas simple optionnel
	* ré-entrelacement `[L, R, Sub]`
10. Extension 5.1 up-mix:
	* `[L, R, C, LFE, Ls, Rs]`
	* C = (L+R)/2, Ls/Rs atténués
	* recalcul `ByteRate = SampleRate * NumChannels * BytesPerSample`
11. Synthèse:
	* génération d'une onde 440 Hz
	* déplacement spatial du son entre canaux 5.1

### Fichiers générés

* `etape00_source_synthetique.wav`
* `etape03_downsample_x2.wav`
* `etape04_quantization_8bit.wav`
* `etape05_06_dessat_norm.wav`
* `etape07_left_channel.wav`
* `etape11_stereo_to_2_1.wav`
* `etape12_stereo_to_5_1.wav`
* `etape13_synth_5_1_travel.wav`