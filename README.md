# snakEFI

The classic arcade game, now as an UEFI executable!

## Running

### Running on real hardware

Download the latest `BOOTX64.EFI` image from the releases page and copy it to a FAT32 formatted thumbdrive at `/EFI/BOOT/BOOTX64.EFI`. Then boot into your computers firmware and select the thumbdrive from the boot options. Alternatively if you'd rather run it inside a VM you can continue reading.

### Running in a VM

To run snakEFI in a VM you need to download the image, install the VM dependencies and then run the following from the same directory as the `BOOTX64.EFI` file:

```bash
dd if=/dev/zero of=snakefi.img bs=1k count=1440
mformat -i snakefi.img -f 1440 ::
mmd -i snakefi.img ::/EFI
mmd -i snakefi.img ::/EFI/BOOT 
mcopy -o -s -i snakefi.img $^ ::/EFI/BOOT
```

this will create a image file called `snakefi.img` which can be run with:

```bash
sudo qemu-system-x86_64 -pflash $UEFI_IMG -drive format=raw,file=snakefi.img
```

where `$UEFI_IMG` is the path to the OMVF firmware bundle, on my system it's `/usr/share/edk2/x64/OVMF_CODE.fd`

#### VM dependencies

- `mtools` (for creating/editing FAT32 images)
- `qemu` (for running program in VM)
- `edk2-ovmf` (UEFI firmware for VM)

## Build

Since UEFI is it's own unique executable format (well... it's not *quite* identical to standard windows PE files) and snakEFI has no runtime dependencies the precompiled version should run but if for whatever reason you want to compile the project yourself the process is pretty straight forward.

### Build Dependencies

- `gnu-efi` (for UEFI headers, not linked)
- `gnumake` (Makefile support)
- `clang` (no `gcc` won't work)
- `lld` (no `ld`/`gold`/`mold` won't work)
