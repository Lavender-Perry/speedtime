{ pkgs ? import (fetchTarball https://github.com/NixOS/nixpkgs/archive/refs/tags/21.05.tar.gz) {} }:

pkgs.stdenv.mkDerivation rec {
  name = "speedtime";
  src = ./src;
  buildInputs = [ pkgs.gcc ];
  builder = ./build.sh;
}
