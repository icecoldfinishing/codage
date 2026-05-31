# Guide : Compression Audio G.711 & DRC

Ce document présente la théorie et l'implémentation de deux types de compression appliquées à nos fichiers WAV : la compression de données logarithmique (norme ITU-T G.711 A-law / $\mu$-law) et la compression de plage dynamique temporelle (DRC).

---

## 1. Compression Logarithmique G.711 (A-law & $\mu$-law)

Dans un fichier WAV standard non compressé (PCM 16 bits linéaire), l'oreille humaine perçoit les variations de pression de manière logarithmique (loi de Weber-Fechner). Cela signifie que la résolution fine sur les sons de très forte amplitude est inutile, tandis qu'une grande précision est nécessaire pour les sons faibles.

La norme **ITU-T G.711** utilise des lois de compression logarithmiques pour coder un échantillon linéaire sur 16 bits en un échantillon logarithmique sur 8 bits seulement, divisant la taille du fichier par 2 tout en conservant une excellente intelligibilité de la parole.

---

### 1.1 La Loi A (A-law) - Utilisée principalement en Europe
Pour un signal d'entrée normalisé $x \in [-1, 1]$, la formule de compression est :
$$F(x) = \operatorname{sgn}(x) \frac{A |x|}{1 + \ln A} \quad \text{pour } |x| < \frac{1}{A}$$
$$F(x) = \operatorname{sgn}(x) \frac{1 + \ln(A |x|)}{1 + \ln A} \quad \text{pour } \frac{1}{A} \le |x| \le 1$$
Où la constante $A = 87.6$.
À la décompression (reconstruction en linéaire), la formule inverse est appliquée.

En pratique, l'implémentation binaire utilise une technique d'approximation par segments (13 segments), évitant les calculs lourds de logarithmes. Chaque échantillon 8 bits résultant contient :
- 1 bit de signe.
- 3 bits indiquant le segment (exposant).
- 4 bits indiquant la position dans le segment (mantisse).
De plus, un XOR avec la valeur `0x55` est appliqué sur l'octet pour stabiliser les lignes de transmission.

---

### 1.2 La Loi $\mu$ ($\mu$-law) - Utilisée principalement en Amérique et au Japon
La formule de compression est définie par :
$$F(x) = \operatorname{sgn}(x) \frac{\ln(1 + \mu |x|)}{\ln(1 + \mu)}$$
Où la constante $\mu = 255$.
Semblable à la loi A, elle compresse sur 8 bits (1 bit de signe, 3 bits de segment, 4 bits de mantisse) avec un XOR de `0xFF` appliqué sur l'octet généré.

La loi $\mu$ offre une dynamique légèrement supérieure pour les signaux de très faible niveau par rapport à la loi A.

---

### 1.3 Comment notre code WAV gère la compression G.711

- **Format WAV généré** : Pour que le fichier WAV soit lisible par un lecteur de musique comme VLC, l'en-tête doit indiquer que les données sont compressées.
  - Champ `audio_format` dans `fmt` : `6` pour A-law, `7` pour $\mu$-law.
  - Champ `bits_per_sample` : `8`.
  - Le `byte_rate` et le `block_align` sont divisés par deux car chaque frame mono ne pèse plus qu'un octet.
- **Pipeline de traitement** : Le signal compressé sur 8 bits ne peut pas subir directement des traitements arithmétiques comme le sous-échantillonnage ou la saturation douce. C'est pourquoi notre pipeline :
  1. Compresse et écrit le fichier compressé sur le disque (réduction de 50%).
  2. Décompresse l'audio pour reconstruire le flux 16 bits linéaire.
  3. Applique les traitements audio (DSP) sur ce flux reconstruit.

---

## 2. Compression de la Plage Dynamique (DRC)

Contrairement à la compression G.711 qui réduit la taille mémoire du fichier, la **compression de dynamique (DRC)** réduit la différence d'intensité entre les parties les plus fortes et les plus faibles du son.

### 2.1 Paramètres clés de notre DRC :
1. **Seuil (Threshold, en dB)** : Le niveau d'amplitude au-dessus duquel la compression commence à agir (ex: $-12$ dB).
2. **Ratio** : Le taux de réduction appliqué au signal dépassant le seuil (ex: $4:1$). Pour un ratio de $4:1$, si le signal dépasse le seuil de 4 dB, il n'en ressortira que de 1 dB au-dessus du seuil.
3. **Gain de compensation (Makeup Gain, en dB)** : Un gain multiplicateur appliqué au signal compressé pour restaurer le volume sonore global (ex: $+3$ dB), rendant les parties initialement faibles beaucoup plus intelligibles et présentes.

### 2.2 Algorithme implémenté :
Pour chaque échantillon $x$ normalisé entre $[-1.0, 1.0]$ :
1. Conversion du seuil en amplitude linéaire : $T = 10^{\frac{\text{seuil}}{20}}$.
2. Si $|x| > T$ :
   $$x_{\text{comp}} = T + \frac{|x| - T}{\text{Ratio}}$$
   $$x = \operatorname{sgn}(x) \cdot x_{\text{comp}}$$
3. Application du gain de compensation linéaire : $G = 10^{\frac{\text{makeup}}{20}}$, d'où $x_{\text{final}} = x \times G$.
4. Clamping de sécurité pour s'assurer que le signal ne dépasse pas $[-1.0, 1.0]$.
