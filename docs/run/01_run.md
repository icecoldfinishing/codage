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
# Note: `make` n'est pas installe par defaut sur Windows.
# Si tu as MSYS2/Chocolatey, installe make ou utilise la compilation manuelle ci-dessous.
make
./codage input.wav

# Compilation manuelle (si pas de Makefile)
gcc -O2 -mavx2 -msse4.1 -Iinclude -o codage.exe src/main.c src/resolution.c -lm
./codage.exe input.wav