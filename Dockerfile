
FROM gcc:15.2

# On définit le dossier de travail à l'intérieur du conteneur
WORKDIR /app

# On copie tes fichiers sources (main.c, exercices.c, exercices.h) dans le conteneur
COPY . .

# On compile le projet
RUN gcc -O2 -mavx2 -o /usr/local/bin/codage main.c resolution.c -lm
# Entrypoint: les arguments passes a `docker run codage ...` sont transmis au binaire.
ENTRYPOINT ["/usr/local/bin/codage"]
# Argument par defaut (peut etre remplace par `docker run codage input.wav`).
CMD ["input.wav"]