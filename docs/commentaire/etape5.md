# Étape 5 – Exportation et Validation

## Objectif

Cette étape consiste à **finaliser le traitement audio** en sauvegardant le fichier WAV modifié et en vérifiant son résultat par une lecture audio.

Elle permet de s’assurer que :

* le fichier généré est valide
* l’en-tête WAV est correctement reconstruit
* le rendu sonore est cohérent après traitement

---

# 9. Sauvegarde du fichier WAV

## Principe

Après tous les traitements (downsampling, quantification, spatialisation, etc.), les données audio sont :

* regroupées dans un tableau d’octets (`byte array`)
* associées à un **nouvel en-tête WAV valide**

---

## Reconstruction de l’en-tête

Lors de la génération du fichier final, il est essentiel de recalculer :

### Taille totale du fichier

```text id="e5exp0"
file_size = 36 + data_size
```

où :

* `36` = taille minimale du header WAV (RIFF + fmt + data header)
* `data_size` = taille des données audio

---

### Débit binaire (ByteRate)

```text id="e5exp1"
ByteRate = SampleRate × BlockAlign
```

Ce champ garantit une lecture correcte du flux audio.

---

### Bloc de données

Les échantillons sont ensuite écrits dans le chunk :

```text id="e5exp2"
data chunk → audio samples
```

---

## Écriture du fichier

Le tableau final est ensuite sauvegardé dans un fichier `.wav` :

```c id="e5exp3"
wav_rebuild_pcm_bytes(dst, &format, data, size);
```

---

## Résultat

* fichier WAV correctement structuré
* compatible avec les lecteurs audio standards
* prêt pour lecture ou export

---

# 10. Test auditif

## Objectif

Vérifier que le traitement audio produit un résultat audible correct.

---

## Lecture du fichier

Le programme tente de lire directement le fichier généré :

```c id="e5exp4"
wav_play_file_simple("outputs/etape03_5_6_process.wav");
```

---

## Fonctionnement

* charge le fichier WAV
* décode les échantillons audio
* joue le son via une bibliothèque audio (ex : `sounddevice`, `pygame`)

---

## Gestion des limitations

Si l’environnement ne permet pas la lecture audio :

```c id="e5exp5"
Lecture non disponible automatiquement sur cet environnement.
```

---

## Résultat attendu

* confirmation que le fichier est lisible
* vérification de la qualité sonore après traitement
* validation du pipeline complet de transformation audio

---

## Conclusion

Cette étape garantit que :

* le fichier WAV est correctement exporté
* les métadonnées sont valides
* le résultat sonore est exploitable
* la chaîne de traitement est complète de bout en bout
