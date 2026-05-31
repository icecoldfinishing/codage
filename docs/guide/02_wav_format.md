# Guide : Structure et Parsing d'un Fichier WAV (Format RIFF)

Ce document explique comment est structuré un fichier audio WAV non compressé (PCM) et comment son en-tête est lu et analysé manuellement en C, sans utiliser de bibliothèque tierce.

---

## 1. Qu'est-ce qu'un fichier WAV ?

Un fichier **WAV** (Waveform Audio File Format) est un conteneur basé sur le format de fichier générique **RIFF** (Resource Interchange File Format) développé par Microsoft et IBM.

Dans un fichier WAV standard, les données audio sont stockées sous forme brute non compressée appelée **PCM** (Pulse Code Modulation).

---

## 2. Structure en Chunks (Blocs) d'un Fichier WAV

Le format RIFF divise le fichier en plusieurs sous-ensembles appelés **chunks**. Chaque chunk a la structure suivante :
- **ID du Chunk** (4 octets) : Code ASCII identifiant le bloc (ex: `RIFF`, `fmt `, `data`).
- **Taille du Chunk** (4 octets, entier Little Endian) : La taille des données qui suivent immédiatement, sans compter les 8 octets d'en-tête du chunk.
- **Données** (taille variable) : Le contenu brut.

Voici la structure générale d'un fichier WAV PCM :

```
┌────────────────────────────────────────────────────────┐
│ Chunk RIFF (Conteneur principal)                       │
│ - ID : "RIFF" (0x46464952)                             │
│ - Taille : (Taille totale du fichier - 8 octets)       │
│ - Format : "WAVE" (0x45564157)                         │
└────────────────────────────────────────────────────────┘
       │
       ├───> ┌──────────────────────────────────────────┐
       │     │ Sub-Chunk "fmt " (Détails du format)     │
       │     │ - ID : "fmt " (0x20746D66)               │
       │     │ - Taille : 16 octets (pour PCM standard) │
       │     │ - AudioFormat : 1 (PCM), 6 (A-law)...     │
       │     │ - NumChannels : 1 (Mono), 2 (Stéréo)...  │
       │     │ - SampleRate : Fréquence (ex: 44100 Hz)  │
       │     │ - ByteRate : Octets par seconde          │
       │     │ - BlockAlign : Octets par frame          │
       │     │ - BitsPerSample : Résolution (8 ou 16)   │
       │     └──────────────────────────────────────────┘
       │
       └───> ┌──────────────────────────────────────────┐
             │ Sub-Chunk "data" (Données audio brutes)  │
             │ - ID : "data" (0x61746164)               │
             │ - Taille : (Taille des échantillons)     │
             │ - Données audio : [échantillon 1, ...]   │
             └──────────────────────────────────────────┘
```

---

## 3. Comprendre les paramètres de l'en-tête `fmt `

Dans notre structure C `WavFormat`, nous stockons les champs clés suivants :

- **`audio_format`** (16 bits) :
  - `1` = PCM linéaire (non compressé).
  - `6` = A-law (logarithmique 8 bits).
  - `7` = $\mu$-law (logarithmique 8 bits).
- **`num_channels`** (16 bits) : Nombre de canaux (1 = mono, 2 = stéréo, 3 = 2.1, 6 = 5.1).
- **`sample_rate`** (32 bits) : Fréquence d'échantillonnage (ex: 44100 Hz, 48000 Hz). Nombre d'échantillons joués par seconde par canal.
- **`block_align`** (16 bits) : Taille en octets d'une seule frame (tous canaux confondus).
  $$\text{block\_align} = \text{num\_channels} \times \frac{\text{bits\_per\_sample}}{8}$$
- **`byte_rate`** (32 bits) : Nombre d'octets de données consommés par seconde.
  $$\text{byte\_rate} = \text{sample\_rate} \times \text{block\_align}$$
- **`bits_per_sample`** (16 bits) : La résolution binaire de chaque échantillon (ex: 8 bits ou 16 bits).

---

## 4. Endianness (Boutisme) et Lecture binaire en C

Les fichiers WAV stockent toutes les valeurs numériques sur plusieurs octets en mode **Little Endian** (l'octet de poids faible en premier).

Par exemple, le nombre `44100` (soit `0xAC44` en hexadécimal) est stocké sur 4 octets sous la forme :
`0x44`, `0xAC`, `0x00`, `0x00`.

Pour lire ces valeurs manuellement en C sans dépendre de l'architecture de la machine hôte, nous utilisons des accesseurs de décalage de bits :

```c
uint16_t lire_u16_le(const uint8_t* p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

uint32_t lire_u32_le(const uint8_t* p) {
    return (uint32_t)(p[0] | ((uint32_t)p[1] << 8) | 
                     ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}
```

---

## 5. Algorithme de Parsing de l'en-tête (dans `wav_parse_header`)

1. **Validation RIFF/WAVE** : On vérifie que les 4 premiers octets sont `"RIFF"` et que les octets 8 à 11 contiennent `"WAVE"`.
2. **Parcours des Chunks** : On commence à l'offset `12`. Pour chaque chunk rencontré :
   - On lit son ID (4 octets) et sa taille $S$ (4 octets).
   - Si l'ID est `"fmt "`, on remplit notre structure de format en lisant les données du bloc.
   - Si l'ID est `"data"`, on mémorise l'offset exact du début des données audio et sa taille $S$.
   - Si c'est un autre chunk (ex: `LIST`, `junk`), on saute simplement sa taille $S$ (arrondie au nombre pair supérieur) pour passer au chunk suivant.
3. **Vérification** : On s'assure que les chunks `fmt ` et `data` ont bien été trouvés et que les paramètres de format sont supportés (ex: formats audio 1, 6 ou 7 et bits_per_sample de 8 ou 16).
