#!/bin/bash
echo "[INFO] Kompiluję main.c..."
gcc main.c -o main
chmod +x main
echo "[INFO] Uruchamiam backend..."
node index.js