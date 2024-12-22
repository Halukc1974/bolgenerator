#!/bin/bash
make clean
make
rm -rf ./Tests
mkdir ./Tests
./scim_bolts SHCS_M014x1000x002 shcs 1.4e-3 3.0e-3 0.3e-3 0.0e-3
./scim_bolts SHCS_M080x1000x002 shcs 7.52e-3 16.5e-3 0.8e-3 0.0e-3
cp ./Tests/*.brep /mnt/c/Users/Samuel\ Jacobs/Documents/