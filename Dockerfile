FROM gcc:15.2

WORKDIR /app

# Installation de make (optionnel mais pratique)
RUN apt-get update && apt-get install -y make && rm -rf /var/lib/apt/lists/*

CMD ["bash"]