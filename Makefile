
# user config
TARGET = BOOTX64.EFI
SRC = main.c utils.c gop.c snake.c rng.c
EXPORTS := $(TARGET)
UEFI_IMG = /usr/share/edk2/x64/OVMF_CODE.fd

# boiler plate config
CC = clang
CFLAGS = -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -I/usr/lib/clang/*/include/ -I/usr/include/efi/ -Wall -Wextra -O3
LDFLAGS = -target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link -flto=full -O3
IMG_FILE = snakefi.img 
OBJDIR = ./obj
OBJFILES := $(addprefix $(OBJDIR)/, $(SRC:.c=.o))

vpath %.c src 

build: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(LDFLAGS) -o $@ $^
	
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(IMG_FILE): $(EXPORTS)
	dd if=/dev/zero of=$(IMG_FILE) bs=1k count=1440
	mformat -i $(IMG_FILE) -f 1440 ::
	mmd -i $(IMG_FILE) ::/EFI
	mmd -i $(IMG_FILE) ::/EFI/BOOT 
	mcopy -o -s -i $(IMG_FILE) $^ ::/EFI/BOOT

$(OBJDIR):
	mkdir -p $(OBJDIR)

run: $(IMG_FILE)
	sudo qemu-system-x86_64 -pflash $(UEFI_IMG) -drive format=raw,file=$(IMG_FILE)

clean:
	rm -vrf $(OBJDIR) $(TARGET) $(IMG_FILE)

.PHONY: build run clean

