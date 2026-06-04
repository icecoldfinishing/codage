# Étape 3 – Traitement du Signal (Arithmétique)

## Objectif

Cette étape consiste à améliorer la qualité audio en deux opérations principales :

* **Gestion de la saturation (soft clipping / désaturation)**
* **Normalisation du volume**

L’objectif est d’éviter la distorsion brutale tout en optimisant le niveau sonore global.

---

# 5. Gestion de la saturation (Soft Desaturation)

## Principe

La saturation apparaît lorsque les échantillons audio atteignent ou dépassent les valeurs maximales possibles.

Au lieu de couper brutalement le signal (clipping dur), on applique une **compression non linéaire douce**.

---

## Détection des pics

```c id="s1sat0"
int32_t peak = max_amp(bps);
```

* `peak` représente la valeur maximale possible selon la quantification (ex : 32767 pour 16 bits).

---

## Transformation non linéaire (tanh)

Le signal est normalisé puis compressé avec une fonction hyperbolique :

```c id="s2sat1"
x = s / peak;
y = tanh(1.8 * x) / tanh(1.8);
```

### Effet :

* les petits signaux restent inchangés
* les grands signaux sont compressés progressivement
* les pics sont arrondis au lieu d’être coupés

---

## Écriture du signal modifié

```c id="s3sat2"
out = (int32_t)llround(y * peak);
```

---

## Détection de saturation

Le code détecte aussi les valeurs proches des limites :

```c id="s4sat3"
if (a >= peak) {
    stats.saturated_count++;
}
```

---

## Résultat

* suppression des saturations brutales
* son plus naturel
* réduction de la distorsion

---

# 6. Normalisation du signal

## Objectif

Augmenter ou réduire le volume global pour utiliser au maximum la plage dynamique sans saturation.

---

## Étape 1 : Recherche du pic global

```c id="n1norm0"
stats.max_abs_before
```

On parcourt tous les échantillons pour trouver l’amplitude maximale.

---

## Étape 2 : Calcul du gain

```c id="n2norm1"
gain =
(peak × target_peak_ratio) /
max_abs_before;
```

### Explication :

* `target_peak_ratio` = marge de sécurité (ex: 0.95)
* permet d’éviter la saturation finale

---

## Étape 3 : Application du gain

```c id="n3norm2"
out = s * gain;
```

Puis on limite les valeurs :

```c id="n4norm3"
clamped = borne_i32(out, -peak, peak);
```

---

## Détection de saturation après normalisation

```c id="n5norm4"
if (clamped != out) {
    stats.saturated_count++;
}
```

---

## Résultat

* volume optimisé
* signal amplifié sans distorsion
* respect des limites physiques du format audio

---

# Résumé global de l’étape 3

## Traitement 1 : Soft Desaturation

* compression non linéaire (tanh)
* réduction des pics extrêmes
* suppression de la distorsion brutale

## Traitement 2 : Normalisation

* recherche du pic maximal
* calcul d’un gain optimal
* amplification contrôlée

---

## Conclusion

Cette étape permet d’obtenir un signal :

* plus propre
* mieux équilibré en volume
* sans saturation excessive
