
# === Stage 1 : Compilation du binaire C ===
FROM gcc:15.2 AS builder

WORKDIR /build
COPY . .

RUN gcc -O2 -mavx2 -msse4.1 -Iinclude \
    -o /usr/local/bin/codage \
    src/main.c src/bases.c src/simd_utils.c src/conversions.c \
    src/wav_core.c src/wav_dsp.c src/wav_compression.c \
    -lm

# === Stage 2 : Serveur GUI Python ===
FROM python:3.12-slim

WORKDIR /app

# Copier le binaire compilé depuis le stage builder
COPY --from=builder /usr/local/bin/codage /usr/local/bin/codage

# Copier uniquement ce dont Flask a besoin
COPY gui/server.py ./gui/server.py
COPY gui/index.html ./gui/index.html

RUN pip install --no-cache-dir flask==3.0.3

EXPOSE 5000

CMD ["python", "gui/server.py"]
