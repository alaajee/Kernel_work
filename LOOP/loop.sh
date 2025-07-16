#!/bin/bash

IMG="/home/jennine/GrapheSimple/loop201.img"
LOOP="/dev/loop201"

# Créer un fichier image de 1Mo rempli de zéros
dd if=/dev/zero of="$IMG" bs=1M count=1 status=none

# Associer le fichier image au loop device
sudo losetup "$LOOP" "$IMG"

# Écrire des données dans le loop device (exemple : écrire "Hello Loop" au début)
echo -n "Hello Looooop" | sudo dd of="$LOOP" bs=1 count=10 conv=notrunc status=none

# Forcer la synchronisation des données sur disque
# sync

# Détacher le loop device proprement
sudo losetup -d "$LOOP"

echo "Écriture et synchronisation terminées sur $IMG via $LOOP"
