# codage

# Projet : Traitement de Données Haute Performance (SIMD & Bitwise)

## Présentation
Ce projet implémente des algorithmes de traitement de données optimisés en langage C. L'objectif est de maximiser l'utilisation des ressources matérielles en exploitant les capacités des processeurs modernes via l'architecture x86_64.

Le code met en œuvre trois piliers fondamentaux de l'informatique de bas niveau :
1. **Manipulation de flux de bits** : Décodage efficace et conversion de bases numériques.
2. **Parallélisme de données (SIMD)** : Utilisation des registres SSE et AVX2 pour traiter plusieurs octets en un seul cycle d'horloge.
3. **Gestion de la mémoire** : Optimisation des accès via le contrôle de l'alignement des données et l'utilisation de fenêtres glissantes.

## Concepts Techniques

### Vectorisation (SIMD)
Au lieu de traiter les données octet par octet, le projet utilise le mode Single Instruction, Multiple Data (SIMD).
* **SSE (128 bits)** : Traitement de 16 octets simultanément.
* **AVX2 (256 bits)** : Traitement de 32 octets simultanément.

### Algorithmes de Masquage
Le projet utilise des techniques de masquage binaire pour :
* Isoler des informations spécifiques via des masques ET (AND).
* Compacter les résultats de comparaisons complexes en masques de bits simples (Movemask).
* Localiser des motifs dans des flux de données sans utiliser de branchements conditionnels (if), évitant ainsi les pénalités de prédiction de branchement.

### Optimisation des Accès Mémoire
Le projet gère les contraintes d'alignement mémoire. L'utilisation stratégique des chargements non-alignés (Unaligned Load) permet de traiter des données contiguës et des décalages d'octets indispensables pour les vérifications d'ordre ou les recherches de sous-séquences.

## Environnement Technique
Le projet nécessite un compilateur supportant les intrinsèques Intel/AMD :
* **Compilateur** : GCC ou Clang.
* **Drapeaux de compilation** : `-mavx2` et `-msse4.1`.

## Performance et Objectifs
* Réduction significative du nombre d'instructions par rapport aux boucles scalaires.
* Augmentation du débit de données pour les opérations de filtrage et de recherche.
* Optimisation de l'empreinte CPU par l'exploitation des unités de calcul vectorielles.