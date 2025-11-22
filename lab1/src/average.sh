./#!/bin/sh

c=0
s=0
for arg in "$@"; do
    s=$((s + arg))
    c=$((c + 1))
    done
s=$((s / c))
echo "Среднее арифметическое $s"