GNU-EFI=./gnu-efi
CC=gcc
CFLAGS=-fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args
INCLUDES=-I$(GNU-EFI)/inc/ -I$(GNU-EFI)/inc/efi -I$(GNU-EFI)/inc/efi/x86_64 -I$(GNU-EFI)/inc/efi/protocol
QEMU=qemu-system-x86_64

.PHONY: clean clean-all

run: boot.img
	${QEMU} -drive file=./ovmf.fd,format=raw,if=pflash -cdrom boot.img

boot.img: main.efi
	dd if=/dev/zero of=boot_aux.img bs=1M count=512
	mkfs.vfat boot_aux.img
	sudo mount boot_aux.img /mnt/
	sudo mkdir -p /mnt/efi/boot/
	sudo cp ./main.efi /mnt/efi/boot/bootx64.efi
	sudo umount /mnt/
	mv boot_aux.img boot.img

main.efi: main.so
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 main.so main.efi

main.so: main.o
	ld -shared -Bsymbolic -L$(GNU-EFI)/x86_64/lib -L$(GNU-EFI)/x86_64/gnuefi -T$(GNU-EFI)/gnuefi/elf_x86_64_efi.lds $(GNU-EFI)/x86_64/gnuefi/crt0-efi-x86_64.o main.o -o main.so -lgnuefi -lefi

main.o: gnu-efi
	$(CC) $(CFLAGS) $(INCLUDES) -c ./src/main.c -o main.o

gnu-efi:
	git clone https://git.code.sf.net/p/gnu-efi/code ${GNU-EFI}
	cd ${GNU-EFI} && make

clean:
	rm -rf *.o *.so *.efi *.img

clean-all: clean
	rm -rf ./gnu-efi