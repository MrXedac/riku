IMAGE=riku.iso
MAKE=make

.PHONY: $(IMAGE) run img

all: $(IMAGE)
	echo "Done !"

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
