{
  description = "Example cmake/c++ flake";
  
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    flake-parts.url = "github:hercules-ci/flake-parts";
    default-systems.url = "github:nix-systems/default";
  };

  outputs = inputs @ { self, nixpkgs, ... }:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      systems = (import inputs.default-systems);
      perSystem = { pkgs, ... }: { 
        packages = let 
          name = "snakEFI";
          version = "1.0.2";
        in rec {
          default = pkgs.stdenv.mkDerivation {
            pname = "${name}";
            inherit version;
            src = self;
            # we're doing some seriously questionable cross-compilation shenanigans
            # and nix's normally great security practices just breaks things.
            hardeningDisable = [ "pic" "stackprotector" ];
            installPhase = ''
              mkdir -p $out/share
              cp BOOTX64.EFI $out/share
            '';
            buildInputs = with pkgs; [
              gnu-efi
              lld_16
              llvmPackages_16.clang
            ];
          };
          image = pkgs.stdenv.mkDerivation {
            pname = "${name}-vm-image";
            inherit version;
            # this is a no-op for unpack, since this derivation has no sources
            unpackPhase = "true";
            buildPhase = ''
              dd if=/dev/zero of=snakEFI.img bs=1k count=1440
              mformat -i snakEFI.img -f 1440 ::
              mmd -i snakEFI.img ::/EFI
              mmd -i snakEFI.img ::/EFI/BOOT 
              mcopy -o -s -i snakEFI.img ${default}/share/BOOTX64.EFI ::/EFI/BOOT
            '';
            installPhase = ''
              mkdir -p $out/share
              cp snakEFI.img $out/share
            '';
            nativeBuildInputs = [
              pkgs.mtools 
              default
            ];
          };
          vm = pkgs.writeShellApplication {
            name = "${name}-vm-wrapper";
            runtimeInputs = [
              pkgs.OVMF
              pkgs.qemu
              default
              image
            ];
            text = ''
              cd "$(realpath "$(dirname "$0")")"
              IMG_TMP="$(mktemp -d)"
              trap 'rm -rf "$IMG_TMP"' EXIT
              cp --no-preserve=mode,ownership ${image}/share/snakEFI.img "$IMG_TMP"
              cp --no-preserve=mode,ownership ${pkgs.OVMF.fd}/FV/OVMF_CODE.fd "$IMG_TMP"
              ${pkgs.qemu}/bin/qemu-system-x86_64 \
                -drive if=pflash,format=raw,file="$IMG_TMP/OVMF_CODE.fd" \
                -drive format=raw,file="$IMG_TMP/snakEFI.img"
            '';
          };
        };
      };
    };
}
