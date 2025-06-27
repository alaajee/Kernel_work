# Liste des modules à compiler
CONFIG_MODULE_SIG=n

ccflags-y += -Wall -g -O3

TARGET = kmodule

kmodule-m += main.o
kmodule-m += SocketHandler.o
kmodule-m += Client.o
kmodule-m += operation.o
kmodule-m += KVS/src/db.o
kmodule-m += KVS/src/io.o
kmodule-m += KVS/src/bucket.o
kmodule-m += KVS/src/bench.o	


obj-m := kmodule.o


# Chemin vers le fichier vmlinux avec debug info pour BTF
KBUILD_VMLINUX := /usr/lib/debug/boot/vmlinux-6.11.0-26-generic
export KBUILD_VMLINUX

# Répertoire courant
PWD := $(CURDIR)

# Compilation des modules avec passage explicite de KBUILD_VMLINUX
all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) KBUILD_VMLINUX=$(KBUILD_VMLINUX) DEBUG_INFO=y modules

# Nettoyage
clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

# Vérification manuelle de la présence du fichier vmlinux
check-btf:
	@if [ -f "$(KBUILD_VMLINUX)" ]; then \
		echo " vmlinux trouvé : $(KBUILD_VMLINUX)"; \
	else \
		echo " vmlinux non trouvé : $(KBUILD_VMLINUX)"; \
		echo "BTF ne sera pas généré."; \
	fi

.PHONY: all clean check-btf
