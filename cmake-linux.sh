#!/bin/bash

cmake -DCMAKE_BUILD_TARGET=buildroot \
	  ..
make
