docker build -t codage .

# Lance avec l'argument par defaut (input.wav)
docker run --rm codage

# PowerShell: montez le dossier courant pour recuperer les sorties WAV sur l'hote
docker run --rm -v "${PWD}:/app" codage input.wav

# CMD (Invite de commandes):
docker run --rm -v "%CD%:/app" codage input.wav

# Alternative robuste (PowerShell/CMD):
docker run --rm -v "D:\L3\Codage\codage:/app" codage input.wav

# Note: le binaire est installe dans /usr/local/bin, donc il reste accessible
# meme quand /app est monte depuis l'hote.

# Execution locale Windows (hors Docker)
gcc -O2 -mavx2 -msse4.1 -o codage.exe main.c resolution.c -lm
.\codage.exe input.wav




docker run --rm -v "${PWD}:/app" codage input.wav