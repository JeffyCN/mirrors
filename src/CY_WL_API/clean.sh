#!/bin/bash
rm ./wl.h
rm ./libwl.a
cd ./libwl
make clean
cd ..
make clean
