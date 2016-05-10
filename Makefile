IMAGE=riku.iso

.PHONY: $(IMAGE) run

all: $(IMAGE)
	echo "Done !"

$(IMAGE):
	cd kern && make clean && make all
	cd loader && make clean && make all
	cp loader/rikuldr.bin boot/boot/rikuldr.bin
	grub-mkrescue -o riku.iso boot/

run:
	qemu-system-x86_64 -boot d -m 4096 -cdrom riku.iso -serial stdio
