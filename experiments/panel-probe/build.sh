#!/usr/bin/env bash
set -euo pipefail
: "${PS3DEV:?}" "${PS3DK:?}"
src=experiments/panel-probe
out=build/panel-probe
mkdir -p "$out" dist
cc=powerpc64-ps3-elf-gcc
for lib in vshtask vshmain; do
  nidgen archive --input "$src/$lib.yaml" --toolchain-bin "$PS3DEV/ppu/bin" \
    --out-dir "$out" --abi ilp32
done
"$cc" -mcpu=cell -std=gnu99 -Os -Wall -Wextra -Werror -fno-builtin \
  -I"$PS3DK/ppu/include" -c "$src/main.c" -o "$out/main.o"
"$cc" -mcpu=cell -specs="$PS3DK/ppu/lib/lv2-prx.specs" \
  -nostdlib -Wl,-q -Wl,--no-warn-rwx-segments -Wl,-T,"$PS3DK/ppu/lib/lv2-prx.ld" \
  "$PS3DK/ppu/lib/lv2-prx-crt.o" "$out/main.o" \
  -L"$out" -L"$PS3DK/ppu/lib" -lvshtask_stub -lvshmain_stub -llv2_stub -lstdc_stub -lgcc \
  -o "$out/panel-probe.elf"
powerpc64-ps3-elf-nm --undefined-only "$out/panel-probe.elf" > "$out/undefined.txt"
test ! -s "$out/undefined.txt"
prx-gen build "$out/panel-probe.elf" -o "$out/panel-probe.prx" --name panel_probe
prx-gen check "$out/panel-probe.prx"
fself "$out/panel-probe.prx" dist/panel-probe.sprx
(cd dist && sha256sum panel-probe.sprx > SHA256SUMS)
