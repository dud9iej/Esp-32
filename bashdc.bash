#!/bin/bash

echo "[CLEANUP] Purging PlatformIO build directories (.pio/)..."
rm -rf .pio

echo "[CLEANUP] Removing temporary build artifacts and object files..."
find . -name "*.o" -type f -delete
find . -name "*.elf" -type f -delete
find . -name "*.bin" -type f -delete

echo "[CLEANUP] Cleaning npm/pip local caches if any..."
pip cache purge > /dev/null 2>&1

echo "[CLEANUP] Silver Parakeet deep clean complete. Space reclaimed!"