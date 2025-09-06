./#!/bin/sh

for i in {1..150}; do
od -A n -t d -N 1 /dev/urandom >> numbers.txt
done