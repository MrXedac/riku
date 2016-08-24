IMAGE=riku.iso

.PHONY: $(IMAGE) run

all: $(IMAGE)
	echo "Done !"

$(IMAGE):
	cd kern && gmake clean && gmake all
	cd loader && gmake clean && gmake all
	cp loader/rikuldr.bin boot/boot/rikuldr.bin
	grub-mkrescue -o riku.iso boot/

run:
	qemu-system-x86_64 -boot d -m 4096 -cdrom riku.iso -serial stdio
