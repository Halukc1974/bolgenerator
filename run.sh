#!/bin/bash
make clean
make
rm -rf ./Tests
mkdir ./Tests
./scim_bolts SHCS_M05x010x200 shcs 5.0 10.0 0.0