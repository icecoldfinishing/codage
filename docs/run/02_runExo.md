# SOLUTION PROPRE (SANS REBUILD IMAGE)

## Projet = Exercices C (SIMD + conversions + bases numériques)

---

## 1. Lancer un conteneur GCC avec ton projet monté

Depuis ton dossier projet :

```bash
docker run --rm -it -v "${PWD}:/app" -w /app gcc:15.2 bash
```

---

## 2. Compiler le projet (UNE seule fois dans le conteneur)

```bash
gcc -O2 -mavx2 -msse4.1 -Iinclude -o codage src/main.c src/bases.c src/simd_utils.c src/conversions.c src/wav_core.c src/wav_dsp.c src/wav_compression.c -lm
```

---

## 3. Exécuter les exercices

```bash
./codage
```
