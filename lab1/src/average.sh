./#!/bin/sh

file="$1"
c=0
s=0
while read arg; do
s=$((s + arg))
c=$((c + 1))
done
s=$((s / c))
echo "Среднее арифметическое $s"