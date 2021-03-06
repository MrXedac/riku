IMAGE=riku.iso
MAKE=make

.PHONY: $(IMAGE) run

all: $(IMAGE)
	echo "Done !"

$(IMAGE):
	cd kern && $(MAKE) clean && $(MAKE) all
	cd loader && $(MAKE) clean && $(MAKE) all
	cp loader/rikuldr.bin boot/boot/rikuldr.bin
	#hdiutil convert boot/boot/rikufs.dmg -format UDTO -o boot/boot/rikufs.bin
	grub-mkrescue -o riku.iso boot/

run:
	qemu-system-x86_64 -boot d -m 4096 -cdrom riku.iso -serial stdio -no-kvm

debug:
	qemu-system-x86_64 -boot d -m 4096 -cdrom riku.iso -serial stdio -S -s
