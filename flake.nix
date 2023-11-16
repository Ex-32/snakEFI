{
  description = "Example cmake/c++ flake";
  
  inputs.nixpkgs.url = "github:NixOS/nixpkgs";

  outputs = { self, nixpkgs }: let
    # supported systems for this package
    supportedSystems = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ];

    # Helper function to generate an attrset '{ x86_64-linux = f "x86_64-linux"; ... }'.
    forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

    # Nixpkgs instantiated for supported system types.
    nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; });
  in {
    packages =  forAllSystems (system: let 
      pkgs = nixpkgsFor.${system};
    in {
      default = pkgs.stdenv.mkDerivation {
        pname = "snakEFI";
        version = "1.0.1";
        src = self;
        # we're doing some seriously questionably cross-compilation shenanigans
        # and nix's normally great security practices just breaks things.
        hardeningDisable = [ "pic" "stackprotector" ];
        installPhase = ''
          mkdir -p $out/share
          cp BOOTX64.EFI $out/share
        '';
        buildInputs = with pkgs; [
          llvmPackages_16.clang
          lld_16
          gnu-efi
        ];
      };
    });
  };
}
