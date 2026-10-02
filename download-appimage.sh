#!/bin/bash

# Script per scaricare AppImage da una release specifica
# Uso: ./download-appimage.sh <versione> [directory]
# Es: ./download-appimage.sh 2.3
# Es: ./download-appimage.sh 2.3 ~/Downloads

set -e

VERSION="${1:?Errore: specificare la versione (es. 2.3)}"
OUTPUT_DIR="${2:-.}"
REPO="gianlucarenzi/RITMO-Music-Tracker"

# Normalizza la versione per il tag
TAG="v${VERSION}"

echo "📥 Scaricando AppImage versione $VERSION dal repository $REPO..."
echo "   Tag: $TAG"
echo "   Destinazione: $OUTPUT_DIR"

# Scarica l'AppImage in una directory temporanea
TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

if ! gh release download "$TAG" --repo "$REPO" --pattern "*AppImage*" --dir "$TEMP_DIR"; then
    echo "❌ Errore: impossibile scaricare la versione $VERSION"
    echo "   Verifica che il tag $TAG esista nel repository"
    exit 1
fi

# Trova il file AppImage scaricato e rinominalo
APPIMAGE_FILE=$(ls "$TEMP_DIR"/*.AppImage 2>/dev/null | head -1)
if [ -z "$APPIMAGE_FILE" ]; then
    echo "❌ Errore: nessun file AppImage trovato nella release $VERSION"
    exit 1
fi

# Determina il nome del file di output
# Ritmo-Linux-... dalla 2.3, RMT-Linux-... fino alla 2.2.1
PREFIX=$(basename "$APPIMAGE_FILE" | sed 's/-Linux.*//')
OUTPUT_FILE="$OUTPUT_DIR/${PREFIX}-Linux-x86_64-${VERSION}.AppImage"

# Sposta il file e rendi eseguibile
mv "$APPIMAGE_FILE" "$OUTPUT_FILE"
chmod +x "$OUTPUT_FILE"

echo "✅ Scaricamento completato!"
echo "   File: $OUTPUT_FILE"
echo "   Dimensione: $(du -h "$OUTPUT_FILE" | cut -f1)"
echo ""
echo "   Lancia con: $OUTPUT_FILE"
