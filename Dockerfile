
FROM gcc:15.2

# On définit le dossier de travail à l'intérieur du conteneur
WORKDIR /app

# On copie les fichiers sources dans le conteneur
COPY . .

# On compile le projet
RUN gcc -O2 -mavx2 -msse4.1 -Iinclude -o /usr/local/bin/codage src/main.c src/resolution.c -lm
# Entrypoint: les arguments passes a `docker run codage ...` sont transmis au binaire.
ENTRYPOINT ["/usr/local/bin/codage"]

CMD ["input.wav"]
