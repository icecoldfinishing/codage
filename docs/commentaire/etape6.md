# Étape 6 – Spatialisation et Synthèse (Extension Multicanaux)

## Objectif général

Cette étape consiste à **transformer et enrichir l’espace sonore** en passant d’un signal stéréo vers des formats multi-canaux (2.1 et 5.1), puis à générer des sons synthétiques avec déplacement spatial.

Les opérations principales sont :

* création de canaux supplémentaires (LFE, Centre, Surround)
* répartition du signal audio dans l’espace
* génération d’ondes sinusoïdales avec mouvement sonore
* mise à jour des métadonnées WAV (channels, byte rate, etc.)

---

# 11. Passage Stéréo (2.0) → 2.1

## Principe

On transforme un signal stéréo en ajoutant un **troisième canal : LFE (Low Frequency Effect)**.

Format final :

```text id="s61c1"
[L, R, Sub]
```

---

## Création du canal LFE

### Calcul de base

```c id="s62c2"
sub = (l + r) / 2;
```

Le canal LFE est obtenu par **moyenne des canaux gauche et droit**, représentant une base sonore commune.

---

## Filtrage basse fréquence (optionnel)

Pour obtenir un vrai canal subwoofer :

### Principe

On applique un filtre passe-bas numérique simple :

```c id="s63c3"
lfe_mem = 0.92 * lfe_mem + 0.08 * sub;
```

### Effet :

* conserve uniquement les basses fréquences
* supprime les variations rapides (aigus)
* stabilise le signal LFE

---

## Ré-entrelacement des canaux

Chaque frame devient :

```text id="s64c4"
[L | R | Sub]
```

---

## Mise à jour de l’en-tête WAV

```c id="s65c5"
num_channels = 3
block_align = 3 × sample_bytes
byte_rate = sample_rate × block_align
```

---

## Résultat

* ajout d’un canal basse fréquence
* enrichissement du rendu spatial
* préparation pour systèmes audio 2.1

---

# 12. Simulation Surround 5.1 (Up-mixing)

## Objectif

Transformer un signal stéréo en un espace sonore **6 canaux :**

```text id="s66c6"
[L, R, C, LFE, Ls, Rs]
```

---

## Répartition spatiale des canaux

### Canal central (C)

```c id="s67c7"
C = (L + R) / 2
```

➡ centre la voix et les éléments principaux

---

### Canal LFE

Filtré en basse fréquence :

```c id="s68c8"
lfe_mem = 0.94 * lfe_mem + 0.06 * C
```

➡ renforce les basses

---

### Canaux surround

```c id="s69c9"
Ls = L / 2
Rs = R / 2
```

➡ ambiance arrière atténuée

---

## Structure finale par frame

```text id="s70c0"
[L | R | C | LFE | Ls | Rs]
```

---

## Mise à jour de l’en-tête WAV

```c id="s71c1"
num_channels = 6
block_align = 6 × sample_bytes
byte_rate = sample_rate × block_align
```

---

## Résultat

* spatialisation complète
* effet surround simulé
* séparation des rôles audio (centre, basses, arrière)

---

# 13. Génération d’ondes sinusoïdales (Synthétiseur)

## Objectif

Créer un signal audio mathématique pur (440 Hz) et le faire **se déplacer dans l’espace 5.1**.

---

## Génération du signal

### Onde sinusoïdale

```c id="s72c2"
s = sin(2π f t)
```

---

## Variation spatiale (position)

On fait évoluer la position du son dans les canaux :

```c id="s73c3"
pos = (n / frame_count) × 5
```

➡ le son “voyage” de gauche à droite

---

## Panning multi-canaux

### Calcul de l’influence par canal

```c id="s74c4"
g = 1 - |ch - pos|
```

---

### Application du volume

```c id="s75c5"
amp = 0.85 × g × s
```

---

## Résultat

* son sinusoïdal pur (440 Hz)
* déplacement progressif entre canaux
* effet de spatialisation dynamique

---

# Conclusion générale de l’étape 6

Cette étape permet de :

## Spatialisation

* passage stéréo → 2.1 → 5.1
* création de LFE, centre et surround
* simulation d’environnement sonore

## Synthèse audio

* génération mathématique de signaux
* contrôle du mouvement sonore
* variation dynamique du volume

## Résultat final

Un système audio complet capable de :

* traiter
* transformer
* spatialiser
* et synthétiser du son en multi-canaux
