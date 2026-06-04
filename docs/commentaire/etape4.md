# Étape 4 – Spatialisation et Synthèse

## 7. Extraction des canaux (Stéréo → Mono)

## Objectif

Cette étape consiste à **séparer les canaux audio** d’un fichier stéréo (ou multi-canaux) afin d’isoler uniquement le **canal gauche** dans un nouveau fichier mono.

Cela permet de :

* traiter indépendamment chaque canal audio
* simplifier le signal
* préparer des effets de spatialisation ou de mixage

---

## Principe de l’audio entrelacé

Dans un fichier stéréo, les échantillons sont stockés de manière **entrelacée** :

```text id="e1st0"
[L1, R1, L2, R2, L3, R3, ...]
```

Chaque frame contient tous les canaux.

---

## Vérification du format

```c id="c1ch0"
if (src->fmt.num_channels < 2u) {
    return -1;
}
```

On s’assure que le fichier est bien stéréo (ou plus).

---

## Lecture des paramètres audio

```c id="c2ch1"
bps = bits_per_sample
sb  = sample_bytes
in_fb = block_align
```

* `sb` : taille d’un échantillon (ex : 2 octets en 16 bits)
* `in_fb` : taille d’une frame complète (L + R + ...)

---

## Calcul du nombre de frames

```c id="c3ch2"
frame_count = data_size / in_fb;
```

Chaque frame représente un instant audio complet.

---

## Création du nouveau format (mono)

```c id="c4ch3"
out_fmt.num_channels = 1;
out_fmt.block_align = sb;
out_fmt.byte_rate = sample_rate × block_align;
```

### Résultat :

* suppression du canal droit
* conservation du canal gauche uniquement

---

## Extraction du canal gauche

```c id="c5ch4"
memcpy(out_data + i * sb, in_frame, sb);
```

### Explication :

* `in_frame` pointe vers une frame complète (L + R)
* le canal gauche est stocké au début
* on copie uniquement la partie gauche

---

## Structure du traitement

Pour chaque frame :

```text id="s1flow"
Frame stéréo :
[L | R]

↓ extraction

Frame mono :
[L]
```

---

## Taille des données de sortie

```c id="c6ch5"
out_data_size = frame_count * sb;
```

Chaque frame ne contient plus qu’un seul canal.

---

## Reconstruction du fichier WAV

```c id="c7ch6"
wav_rebuild_pcm_bytes(dst, &out_fmt, out_data, out_data_size);
```

Le fichier final devient un WAV mono valide.

---

## Résultat

* suppression du canal droit
* fichier plus léger
* signal simplifié
* prêt pour traitement spatial ou mixage

---

## Conclusion

Cette étape permet de :

* transformer un signal stéréo en mono
* isoler un canal spécifique (ici gauche)
* préparer des opérations de spatialisation avancée
