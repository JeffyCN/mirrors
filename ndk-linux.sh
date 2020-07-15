#!/bin/bash

cmake -DDEBUG=ON -DCMAKE_BUILD_TARGET=buildroot \
	  ..
make
