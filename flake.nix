{
  description = "PlatformIO project development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = self;

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            platformio
            gcc
            python3
          ];
          shellHook = ''
            export PLATFORMIO_CORE_DIR=$PWD/.platformio
            echo "PlatformIO environment ready. Run 'pio run' to build."
          '';
        };
      }
    );
}
