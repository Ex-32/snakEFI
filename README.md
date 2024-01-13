# snakEFI

The classic arcade game, now as a bare bones UEFI executable!

## Running

### Running on real hardware

Download the latest `BOOTX64.EFI` image from the releases page and copy it to a FAT32 formatted thumbdrive at `/EFI/BOOT/BOOTX64.EFI`. Then boot into your computers firmware and select the thumbdrive from the boot options.

### Running in a VM

nix users, you can download, build, and run a VM image with:

```bash
nix run 'github:Ex-32/snakEFI#vm'
```

for everyone else...

#### VM dependencies

- `mtools` (for creating/editing FAT32 images)
- `qemu` (for running program in VM)
- `edk2-ovmf` (UEFI firmware for VM)M dependencies

#### Creating the VM image

To run snakEFI in a VM you need to download the image, install the VM dependencies and then run the following from the same directory as the `BOOTX64.EFI` file:

```bash
dd if=/dev/zero of=snakEFI.img bs=1k count=1440
mformat -i snakEFI.img -f 1440 ::
mmd -i snakEFI.img ::/EFI
mmd -i snakEFI.img ::/EFI/BOOT
mcopy -o -s -i snakEFI.img BOOTX64.EFI ::/EFI/BOOT
```

#### Running the VM image

this will create a image file called `snakEFI.img` which can be run with:

```bash
sudo qemu-system-x86_64 -drive if=pflash,format=raw,file=$UEFI_IMG -drive format=raw,file=snakEFI.img
```

where `$UEFI_IMG` is the path to the OMVF firmware bundle, on my system it's `/usr/share/edk2/x64/OVMF_CODE.fd`

## Building

Since UEFI Applications are their own unique executable format (well... it's not _quite_ identical to standard windows PE files) handled by the system firmware rather than any kernel, and snakEFI has no runtime dependencies, the precompiled version should run on any amd64, UEFI capable machine, but if for whatever reason you want to compile the project yourself, the process is pretty straight forward.

### Nix

if you're using nix, you can build this project by just running one of these:

```bash
nix build 'github:Ex-32/snakEFI'       # BOOTX64.EFI
nix build 'github:Ex-32/snakEFI#image' # snakEFI.img
nix build 'github:Ex-32/snakEFI#vm'    # snakEFI-vm-wrapper
```

### Build Dependencies

- `gnu-efi` (for UEFI headers, not linked)
- `gnumake` (probably packaged by your distro as just `make`)
- `clang` (no `gcc` won't work)
- `lld` (no `ld`/`gold`/`mold` won't work)

### Build Process

First, clone the repository.

```bash
git clone https://github.com/Ex-32/snakEFI.git && cd snakEFI
```

To just create the EFI Application binary, for example to [run on hardware](https://github.com/Ex-32/snakefi/#running-on-real-hardware) just run:

```bash
make
```

this creates the `BOOTX64.EFI` binary in the project directory.

If your goal is to [run snakEFI in a VM](https://github.com/Ex-32/snakefi/#running-in-a-vm), then make sure you've got the [VM dependencies](https://github.com/Ex-32/snakefi/#vm-dependencies) installed and run:

```bash
make run
```

this will compile the project, create the VM image, and run it using `qemu`; if `qemu` exits with an error you may need to adjust the value of `UEFI_IMG` at the top of the `Makefile` to where `OVMF_CODE.fd` is actually located on your system.

If you just want to create the VM image but not run it you can run:

```bash
make img
```

which will create the `snakEFI.img` file, which can then be run manually as described [here](https://github.com/Ex-32/snakefi/#running-the-vm-image) or by running `make run`
