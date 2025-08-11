#!/bin/bash

# Répertoire contenant les images
IMG_DIR="/home/alaa/Kernel_work/LOOP"

# Étape 1 : Libérer les anciens loop devices (s'ils sont encore attachés)
echo "🔄 Détachement des anciens loop devices..."
for i in $(seq 18 1000); do
    dev="/dev/loop$i"
    sudo losetup -d "$dev" 2>/dev/null && echo "Détaché $dev"
done

# Étape 2 : Suppression des anciens fichiers images
echo "🗑️ Suppression des anciens fichiers .img..."
for i in $(seq 18 1000); do
    sudo rm -f "$IMG_DIR/loop$i.img"
done

# Étape 3 : Création des nouveaux fichiers .img et attachement automatique
echo "🆕 Création et attachement des nouvelles images..."
for i in $(seq 18 1000); do
    img_path="$IMG_DIR/loop$i.img"

    # Créer une image vide de 1 Mo (bs=1M count=1 suffit)
    dd if=/dev/zero of="$img_path" bs=1M count=1 status=none

    # Associer l'image à un device loop disponible
    sudo losetup "/dev/loop$i" "$img_path"
done

# Forcer la synchronisation des données sur disque
sync

echo "✅ Tous les loop devices sont créés et synchronisés."
