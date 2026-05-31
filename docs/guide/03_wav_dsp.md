# Guide : Traitement Numérique du Signal Audio (DSP WAV)

Ce document explique les concepts mathématiques et algorithmiques mis en œuvre pour modifier, saturer, sous-échantillonner, quantifier et spatialiser nos données audio WAV.

---

## 1. Accès aux échantillons : Représentation Signée

Selon le nombre de bits par échantillon, l'audio PCM stocke ses données différemment :
- **8 bits** : Non signé. Les valeurs vont de `0` à `255`, où le silence (zéro physique) est situé à `128` (offset constant). Pour obtenir un signal centré sur zéro entre `-128` et `+127`, il faut soustraire `128` à la lecture.
- **16 bits** : Signé en complément à deux. Les valeurs vont de `-32768` à `+32767`, et le silence est bien à `0`. Les données sont stockées en Little Endian (2 octets par échantillon).

Pour lire et écrire ces données de manière transparente, nous utilisons les accesseurs suivants :
- **Lecture** : Ramène l'échantillon à une valeur signée cohérente (de $-128$ à $+127$ pour 8 bits, ou $-32768$ à $+32767$ pour 16 bits).
- **Écriture** : Applique un clamping (écrêtage de sécurité) et effectue la conversion inverse (ajout de `128` pour 8 bits).

---

## 2. Sous-échantillonnage par 2 (Downsampling)

Le sous-échantillonnage divise la fréquence d'échantillonnage par 2 (ex: de 44100 Hz à 22050 Hz). Pour chaque paire d'échantillons consécutifs dans le temps ($x_{2n}$ et $x_{2n+1}$), nous devons générer un unique échantillon de sortie $y_n$.

Nous proposons deux méthodes de fusion :
1. **Moyenne Temporelle** (Filtre décimateur passe-bas de base) :
   $$y_n = \frac{x_{2n} + x_{2n+1}}{2}$$
   Cela réduit l'aliasing (repliement de spectre) en lissant le signal.
2. **Maximum en Valeur Absolue** (Conserve les pics d'attaque) :
   $$y_n = \text{si } |x_{2n}| \ge |x_{2n+1}| \text{ alors } x_{2n} \text{ sinon } x_{2n+1}$$

L'en-tête du fichier de sortie est ensuite mis à jour en divisant `sample_rate` et `byte_rate` par 2.

---

## 3. Quantification 16 bits vers 8 bits

La quantification consiste à réduire la résolution en bits de chaque échantillon. Passer de 16 bits à 8 bits divise par 256 le nombre de niveaux d'amplitude possibles (de 65536 à 256), réduisant ainsi la taille du flux audio au détriment de l'apparition d'un bruit de quantification (souffle).

L'équation de conversion de l'échantillon 16 bits signé $S_{16} \in [-32768, 32767]$ vers l'échantillon 8 bits non signé $U_8 \in [0, 255]$ est :
$$U_8 = \text{clamp}\left( \frac{S_{16} + 32768}{256}, 0, 255 \right)$$
En C, cela s'implémente efficacement par un décalage de bits vers la droite après décalage de la plage de valeurs :
```c
int32_t u8 = ((int32_t)s16 + 32768) >> 8;
```

---

## 4. Dé-saturation par Soft-Clipping (`tanh`)

Lorsque l'amplitude d'un signal dépasse le niveau maximum autorisé (`peak` = 32767 en 16 bits), l'écrêtage brutal (hard clipping) produit une distorsion harmonique très désagréable (clipping numérique).

Pour adoucir cela, nous appliquons une fonction de transfert non linéaire compressive basée sur la tangente hyperbolique (`tanh`) :
1. Normalisation de l'échantillon $x = \frac{s}{\text{peak}} \in [-1, 1]$.
2. Application de la compression douce :
   $$y = \frac{\tanh(1.8 \cdot x)}{\tanh(1.8)}$$
   *Note : Le facteur $1.8$ contrôle l'intensité de la compression de courbe. La division par $\tanh(1.8)$ assure que le signal compressé atteint bien le peak de $1.0$ sans le dépasser.*
3. Restitution : $s_{\text{out}} = y \times \text{peak}$.

Cette courbe compresse les grands pics d'amplitude de façon progressive, imitant le comportement des amplificateurs analogiques ou des bandes magnétiques.

---

## 5. Normalisation

La normalisation consiste à ajuster le volume global du fichier audio pour que son pic de niveau atteigne exactement un ratio cible du maximum absolu (par exemple 95% ou `0.95`).
1. **Recherche du pic** : On parcourt tout le signal pour trouver l'échantillon de valeur absolue maximale $M$.
2. **Calcul du gain** :
   $$\text{gain} = \frac{\text{peak} \times 0.95}{M}$$
3. **Application du gain** : On multiplie chaque échantillon par ce gain fixe.

---

## 6. Upmix Multicanaux : Extraction, 2.1 et 5.1

Les signaux stéréo interlacés stockent alternativement les canaux gauche ($L$) et droit ($R$) : `[L1, R1, L2, R2, ...]`.

### Extraction canal gauche
On prend simplement un échantillon sur deux et on reconstruit un fichier avec `num_channels = 1`.

### Upmix 2.1
- **Canal Gauche (L)** : Copie conforme du canal gauche d'origine.
- **Canal Droit (R)** : Copie conforme du canal droit d'origine.
- **Caisson de Basses (LFE - Low Frequency Effects)** : Somme des canaux gauche et droit :
  $$\text{Sub} = \frac{L + R}{2}$$
  Puis application d'un filtre passe-bas du premier ordre très simple (filtre de moyenne glissante récursive) pour ne garder que les basses fréquences (sous 120 Hz) :
  $$\text{lfe\_mem} = 0.92 \times \text{lfe\_mem} + 0.08 \times \text{Sub}$$

### Upmix 5.1
- **Canal Centre (C)** : Créé pour stabiliser les voix : $C = \frac{L + R}{2}$.
- **Canal LFE (Caisson)** : Filtré passe-bas à partir du canal Centre.
- **Canaux Surround Gauche/Droit (Ls/Rs)** : Effets arrière créés par atténuation et léger déphasage :
  $$Ls = \frac{L}{2}, \quad Rs = \frac{R}{2}$$
Les données de sortie sont entrelacées selon l'ordre standard : `[L, R, C, LFE, Ls, Rs]`.
