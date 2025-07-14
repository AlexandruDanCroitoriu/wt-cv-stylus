#!/bin/bash

# Whisper Model Download Script
# This script downloads Whisper models for speech-to-text functionality

echo "Downloading Whisper models..."

# Create models directory if it doesn't exist
mkdir -p models

# Change to models directory
cd models

# Download base English model (good balance of speed and accuracy)
echo "Downloading base English model (74MB)..."
wget -c https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en.bin

# Optionally download other models (uncomment as needed)

# Tiny model (fastest, least accurate - 39MB)
# echo "Downloading tiny model..."
# wget -c https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.bin

# Small model (good balance - 244MB)
# echo "Downloading small model..."
# wget -c https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin

# Medium model (better accuracy - 769MB) 
# echo "Downloading medium model..."
# wget -c https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-medium.bin

# Large model (best accuracy, slowest - 1550MB)
# echo "Downloading large model..."
# wget -c https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-large-v3.bin

echo "Model download complete!"
echo ""
echo "Models downloaded:"
ls -lh *.bin 2>/dev/null || echo "No models found"
echo ""
echo "To use different models, uncomment the desired wget commands in this script."
