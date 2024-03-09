#!/bin/bash

# mostly copied from cibicc here:
# https://github.com/rui314/chibicc/commit/0522e2d77e3ab82d3b80a5be8dbbdc8d4180561c

assert() {
  expected="$1"
  input="$2"

  echo ""

  ./gosie "$input" > tmp.asm || exit
  customasm -q -f binary -o tmp.rom cpudefs/rj32_cpudef.asm tmp.asm
  ./emu/rj32/emurj tmp.rom
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42
assert 21 '5+20-4'
assert 3 '12-14+5'
assert 41 ' 12 + 34 - 5 '

echo ""
echo ALL PASS