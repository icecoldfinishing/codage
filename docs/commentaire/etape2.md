# Étape 2 – Changement d’échantillonnage (Downsampling ×2)

## Objectif

Réduire la fréquence d’échantillonnage d’un fichier WAV par **2** afin de diminuer la quantité de données audio tout en conservant un signal exploitable.

Exemple :

```text
44100 Hz → 22050 Hz
48000 Hz → 24000 Hz
```

---

## Principe

Le traitement parcourt les données audio **deux frames à la fois**.

Pour chaque paire :

```text
Frame1 + Frame2
↓
Nouvelle Frame
```

Le traitement est réalisé indépendamment pour chaque canal.

Exemple stéréo :

```text
[L1,R1] [L2,R2]
↓
[Lmix,Rmix]
```

---

## Méthodes de réduction

### Méthode 1 : Moyenne des échantillons

Formule :

```text
nouveau = (v1 + v2) / 2
```

Exemple :

```text
1000 1200
↓
1100
```

Avantage :

* son plus lisse
* réduction des variations brutales

Code :

```c
mix = (v1 + v2) / 2;
```

---

### Méthode 2 : Conservation du maximum

Formule :

```text
nouveau =
|v1| ≥ |v2|
? v1
: v2
```

Exemple :

```text
500 -900
↓
-900
```

Avantage :

* conserve davantage les pics du signal
* préserve certaines attaques sonores

Code :

```c
mix =
(abs_i32(v1) >= abs_i32(v2))
? v1
: v2;
```

---

## Calcul du nouveau fichier

### Nombre de frames

```text
FramesSortie =
FramesEntrée / 2
```

---

### Nouvelle fréquence

```text
SampleRate =
AncienSampleRate / 2
```

Code :

```c
out_fmt.sample_rate =
src->fmt.sample_rate / 2;
```

---

### Nouveau ByteRate

Formule :

```text
ByteRate =
SampleRate × BlockAlign
```

Code :

```c
out_fmt.byte_rate =
out_fmt.sample_rate *
out_fmt.block_align;
```

---

## Reconstruction du fichier

Une fois les nouvelles données générées :

```c
wav_rebuild_pcm_bytes(
    dst,
    &out_fmt,
    out_data,
    out_data_size
);
```

Le programme génère un nouveau fichier WAV valide.

---

## Résultat

* fréquence divisée par 2
* fichier plus léger
* moins de détails audio
* même nombre de canaux
* même quantification

# Étape 2 – Changement de quantification (16 bits → 8 bits)

## Objectif

Réduire la résolution du son en transformant chaque échantillon audio de **16 bits** vers **8 bits** afin de diminuer la taille du fichier tout en conservant la forme générale du signal.

## Principe

Le programme parcourt tous les échantillons du fichier WAV et convertit chaque amplitude :

* Entrée : entier signé **16 bits**

  ```
  [-32768 ; +32767]
  ```

* Sortie : entier non signé **8 bits**

  ```
  [0 ; 255]
  ```

La conversion est réalisée avec :

```c
u8 = (s16 + 32768) >> 8;
```

## Explication de la transformation

### Décalage de plage

```c
s16 + 32768
```

Transforme :

```
[-32768 ; +32767]
↓
[0 ; 65535]
```

Cela permet de convertir les amplitudes négatives vers un intervalle positif.

### Réduction de précision

```c
>> 8
```

Décalage binaire vers la droite de 8 bits.

Équivalent à :

```text
division par 256
```

Cette opération supprime les bits de poids faible afin de passer de **16 bits** à **8 bits**.

Exemple :

```
32767 → 255
0 → 128
-32768 → 0
```

### Limitation des valeurs

```c
borne_i32(u8, 0, 255)
```

Garantit que la valeur finale reste valide pour un échantillon 8 bits.

## Mise à jour de l’en-tête WAV

Après conversion :

### Nouvelle quantification

```c
out_fmt.bits_per_sample = 8;
```

### Nouveau BlockAlign

```c
out_fmt.block_align =
    out_fmt.num_channels;
```

Formule :

```text
BlockAlign =
NombreCanaux × OctetsParÉchantillon
```

### Nouveau ByteRate

```c
out_fmt.byte_rate =
    out_fmt.sample_rate *
    out_fmt.block_align;
```

Formule :

```text
ByteRate =
SampleRate × BlockAlign
```

## Reconstruction du fichier

Une fois les échantillons convertis :

```c
wav_rebuild_pcm_bytes(
    dst,
    &out_fmt,
    out_data,
    out_data_size
);
```

Le programme reconstruit un nouveau fichier WAV valide contenant les données audio quantifiées.

## Résultat

* Taille du fichier réduite
* Qualité sonore diminuée
* Forme générale de l’onde conservée
* Compatible avec les lecteurs WAV
