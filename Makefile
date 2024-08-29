IMAGE=riku.iso
MAKE=make
CONF=./kconfig/frontends/conf/conf
MCONF=./kconfig/frontends/mconf/mconf
KCONFIG_AUTOHEADER=kern/include/kconfig.h

.PHONY: $(IMAGE) run img mount umount menuconfig config

all: $(IMAGE)
	echo "Done !"

menuconfig:
	$(MCONF) KConfig

config:
	mkdir -p include/
	mkdir -p include/config 
	KCONFIG_AUTOHEADER=$(KCONFIG_AUTOHEADER) $(CONF) --silentoldconfig KConfig

$(IMAGE):
	cd kern && $(MAKE) clean && $(MAKE) all
	cd loader && $(MAKE) clean && $(MAKE) all
	cp loader/rikuldr.bin boot/boot/rikuldr.bin
	#hdiutil convert boot/boot/rikufs.dmg -format UDTO -o boot/boot/rikufs.bin
	grub-mkrescue -o riku.iso boot/

img:
	qemu-img create -f raw hda.img 4G

run:
	qemu-system-x86_64 -boot d -m 4096 -cdrom riku.iso -hda hda.img -serial stdio 

debug:
	qemu-system-x86_64 -boot d -m 4096 -cdrom riku.iso -hda hda.img -serial stdio -S -s

mount:
	sudo losetup -Pf hda.img

umount:
	sudo losetup -d /dev/loop0
