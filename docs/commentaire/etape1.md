# Étape 1 – Analyse de la structure (Parsing)

## Objectif

Lire l’en-tête du fichier WAV afin d’extraire les informations nécessaires au traitement du signal audio.

Le programme affiche les informations principales contenues dans la structure RIFF/WAVE ainsi que l’emplacement exact des données audio.

---

## Lecture de l’en-tête (Header)

La fonction lit les champs du bloc **fmt** du fichier WAV.

Code :

```c id="l8rl6j"
printf(
"WAV: sampleRate=%u Hz, channels=%u, bits=%u, byteRate=%u, blockAlign=%u"
);
```

Les informations extraites sont :

### Fréquence d’échantillonnage

```text id="jzw9yn"
sample_rate
```

Nombre d’échantillons enregistrés par seconde.

Exemple :

```text id="chq8su"
44100 Hz
48000 Hz
```

Plus cette valeur est élevée, meilleure est la qualité sonore.

---

### Nombre de canaux

```text id="vnmnww"
num_channels
```

Détermine le nombre de pistes audio.

Exemple :

```text id="ls8q0q"
1 → Mono
2 → Stéréo
6 → Surround 5.1
```

---

### Quantification

```text id="0tvhgo"
bits_per_sample
```

Nombre de bits utilisés pour stocker un échantillon.

Exemple :

```text id="5e8pr2"
8 bits
16 bits
24 bits
32 bits
```

Plus cette valeur est élevée, plus la précision des amplitudes est grande.

---

### Débit de données

```text id="5q1b6t"
byte_rate
```

Nombre d’octets lus chaque seconde.

Formule :

```text id="l2i5nl"
ByteRate =
SampleRate × BlockAlign
```

---

### Taille d’une frame

```text id="j5w9q0"
block_align
```

Nombre d’octets nécessaires pour stocker un instant audio complet.

Formule :

```text id="7ffu5v"
BlockAlign =
Canaux × BitsParÉchantillon / 8
```

---

## Identification des données audio

Le programme localise le début du bloc :

```text id="h4wkrm"
data
```

Informations affichées :

```c id="udk41m"
dataOffset
dataSize
```

### Position des données

```text id="g6rq1i"
dataOffset
```

Adresse où commencent réellement les échantillons audio dans le tableau d’octets.

Exemple :

```text id="e2t4bo"
44 octets
```

(cas classique WAV PCM)

---

### Taille des données audio

```text id="0s2z5v"
dataSize
```

Nombre total d’octets contenant les échantillons.

---

## Résultat

La fonction permet de :

* lire les métadonnées du fichier WAV ;
* récupérer la fréquence d’échantillonnage ;
* récupérer le nombre de canaux ;
* récupérer la quantification ;
* localiser précisément le début des données audio ;
* préparer les traitements des étapes suivantes.
