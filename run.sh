#!/bin/bash
make clean
make
rm -rf ./Tests
mkdir ./Tests

./scim_bolts SHCS_M5x0.8_10mm 5.0e-3 8.0e-4 1.0e-2 8.5e-3 5.0e-3 4.0e-3

cp ./Tests/*.brep /mnt/c/Users/Samuel\ Jacobs/Documents/