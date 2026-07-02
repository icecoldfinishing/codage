#!/usr/bin/env python3
"""
Serveur Flask pour l'interface graphique de traitement WAV.
Lance le binaire C via subprocess et streame les logs en temps réel.
"""

import os
import subprocess
import tempfile
import shutil
from flask import Flask, request, jsonify, send_from_directory, Response
from pathlib import Path

app = Flask(__name__, static_folder=".")

# Dossier de travail : le répertoire racine du projet (parent de gui/)
PROJECT_DIR = Path(__file__).parent.parent.resolve()

# Dans Docker, le binaire est installé dans /usr/local/bin/codage
if os.path.exists("/usr/local/bin/codage"):
    BINARY = "/usr/local/bin/codage"
else:
    BINARY = str(PROJECT_DIR / "codage.exe") if os.name == "nt" else str(PROJECT_DIR / "codage")


@app.route("/")
def index():
    return send_from_directory(".", "index.html")


@app.route("/download/<filename>")
def download(filename):
    """Télécharger un fichier généré."""
    # Assurer que le fichier commence par etape pour des raisons de sécurité
    if not filename.startswith("etape"):
        return jsonify({"error": "Accès refusé"}), 403
    return send_from_directory(str(PROJECT_DIR / "outputs"), filename, as_attachment=True)


@app.route("/process", methods=["POST"])
def process():
    """Upload d'un fichier WAV et traitement via le binaire C."""
    if "file" not in request.files:
        return jsonify({"error": "Aucun fichier fourni"}), 400

    f = request.files["file"]
    if not f.filename.lower().endswith(".wav"):
        return jsonify({"error": "Seuls les fichiers .wav sont acceptés"}), 400

    # Sauvegarde temporaire dans le répertoire projet pour que le binaire y accède
    tmp_input = PROJECT_DIR / "input_gui.wav"
    f.save(str(tmp_input))

    # S'assurer que le répertoire de sortie existe
    (PROJECT_DIR / "outputs").mkdir(exist_ok=True)

    def generate():
        """Générateur SSE : streame chaque ligne de stdout comme un événement."""
        yield _sse("start", "Démarrage du traitement...")

        try:
            # On invoque directement le binaire compilé (déjà sur le système)
            proc = subprocess.Popen(
                [BINARY, str(tmp_input)],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                cwd=str(PROJECT_DIR),
            )

            for line in proc.stdout:
                line = line.rstrip()
                if not line:
                    continue
                # Classification de la ligne
                if any(k in line for k in ["Cree", "cree", "Compression", "Decompression",
                                            "Parsing", "WAV:", "Resultat", "Signed", "IEEE",
                                            "Entier", "Decimal", "Exercice", "Conversion"]):
                    yield _sse("success", line)
                elif any(k in line for k in ["Echec", "echec", "ignoree", "invalide",
                                               "impossible", "Erreur", "erreur"]):
                    yield _sse("warning", line)
                elif "---" in line or "===":
                    yield _sse("section", line)
                else:
                    yield _sse("info", line)

            proc.wait()
            if proc.returncode == 0:
                yield _sse("done", "Traitement terminé avec succès")
            else:
                yield _sse("error", f"Erreur : code de sortie {proc.returncode}")

        except FileNotFoundError:
            yield _sse("error", f"Binaire introuvable : {BINARY}. Compilez d'abord avec make ou Docker.")
        except Exception as e:
            yield _sse("error", f"Exception inattendue : {str(e)}")

    return Response(generate(), mimetype="text/event-stream",
                    headers={"Cache-Control": "no-cache", "X-Accel-Buffering": "no"})


@app.route("/outputs")
def outputs():
    """Liste les fichiers WAV générés dans le répertoire outputs/."""
    wavs = []
    outputs_dir = PROJECT_DIR / "outputs"
    if outputs_dir.exists():
        for p in sorted(outputs_dir.glob("etape*.wav")):
            size_kb = round(p.stat().st_size / 1024, 1)
            wavs.append({"name": p.name, "size_kb": size_kb})
    return jsonify(wavs)


def _sse(event: str, data: str) -> str:
    return f"event: {event}\ndata: {data}\n\n"


if __name__ == "__main__":
    port = int(os.environ.get("PORT", 5000))
    print(f"🎵 Interface WAV démarrée sur http://localhost:{port}")
    app.run(host="0.0.0.0", port=port, debug=True, threaded=True)
