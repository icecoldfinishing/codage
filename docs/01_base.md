# Aide-mémoire : Syntaxes et Opérations Bas-Niveau (C/SIMD)

## 1. Opérateurs Binaires de Base
Indispensables pour la conversion octale et le décodage.

| Opération | Syntaxe | Usage dans le projet |
| :--- | :--- | :--- |
| **ET (AND)** | `a & b` | Isoler des bits (ex: `temp & 0x7` pour l'octal). |
| **OU (OR)** | `a \| b` | Combiner des bits (ex: `registre \| bit` pour l'encodage). |
| **Décalage Droite** | `a >> n` | Supprimer les bits traités ou aligner un bit spécifique. |
| **Décalage Gauche** | `a << n` | Faire de la place pour un nouveau bit ou aligner le padding. |

---

## 2. Programmation SIMD (AVX2 & SSE)
Nécessite `#include <immintrin.h>`.

### Types de données
* `__m256i` : Registre de 256 bits (32 octets), utilisé pour AVX2.
* `__m128i` : Registre de 128 bits (16 octets), utilisé pour SSE.

### Chargement et Stockage (LOD/STR)
Le suffixe **`u`** signifie *Unaligned* (évite les crashs si l'adresse n'est pas multiple de 16 ou 32).
* `_mm256_loadu_si256` : Charge 32 octets dans un registre AVX2.
* `_mm_loadu_si128` : Charge 16 octets dans un registre SSE.
* `_mm256_storeu_si256` : Écrit le contenu d'un registre AVX2 en mémoire.

### Initialisation et Comparaison
* `_mm_set1_epi8(val)` : Remplit toutes les cases du registre avec la même valeur `val`.
* `_mm256_cmpgt_epi8(A, B)` : Compare `A > B` (Octets de 8 bits). Retourne `0xFF` si vrai, `0x00` si faux.
* `_mm_cmpeq_epi8(A, B)` : Compare `A == B` (Octets de 8 bits). Retourne `0xFF` si vrai, `0x00` si faux.

---

## 3. Analyse de Masques (Movemask)
Technique pour transformer un registre SIMD en un entier manipulable.

* `_mm_movemask_epi8(cmp)` :
    * Prend le bit de poids fort (MSB) de chaque octet du registre `cmp`.
    * Les concatène dans un `int` (16 bits pour SSE, 32 bits pour AVX).
* `__builtin_ctz(mask)` :
    * *Count Trailing Zeros*.
    * Donne l'index du premier bit à `1` en partant de la droite.
    * Très utile pour convertir un masque en index de tableau.

---

## 4. Astuces de Formatage (printf)
* `%d` : Entier décimal.
* `0x%02X` : Hexadécimal sur 2 caractères, majuscules, avec un zéro initial (ex: `0x0A`).
* `0x%04X` : Hexadécimal sur 4 caractères (ex: `0x0020`).

---

## 5. Logique de l'Exercice 1 (Huffman)
* **Décodage** : On lit de **gauche à droite** (MSB vers LSB) de l'octet source pour construire le registre.
* **Encodage** : On extrait les bits du code du dictionnaire et on les pousse dans un buffer.
* **Padding** : Si `bits_utilises < 8` à la fin, on fait `buffer << (8 - bits_utilises)` pour aligner les bits significatifs vers la gauche.