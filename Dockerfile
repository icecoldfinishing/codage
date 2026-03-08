# On utilise une image officielle qui contient déjà GCC (le compilateur C)
FROM gcc:latest

# On définit le dossier de travail à l'intérieur du conteneur
WORKDIR /app

# On copie tes fichiers sources (main.c, exercices.c, exercices.h) dans le conteneur
COPY . .

# On compile le projet
RUN gcc -o codage main.c resolution.c

# On dit au conteneur de lancer le programme à la fin
CMD ["./codage"]