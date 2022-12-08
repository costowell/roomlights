#!/bin/sh

cd arduino
make
cd ../computer
make clean_debug
cd ..
./computer/roomlights
