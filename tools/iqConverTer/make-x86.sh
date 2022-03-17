#!/bin/bash

set -e

if [ ! -d "build/release" ]; then
    mkdir -p "build/release"
else
    echo "clean"
    # rm build/* -r
fi

cd build

cmake -DCMAKE_INSTALL_PREFIX=./release -DISP_HW_VERSION=${ISP_HW_VERSION} ../ \
&& make -j4 \
&& make install

if [ $? -eq 0  ]; then
    echo Build finished!
    cd ..
fi

BIN=$(pwd)/build/release/bin/iqConverTer
if [ "-DISP_HW_V21" == "${ISP_HW_VERSION}" ]; then
  pushd ../../iqfiles/isp21
else
  pushd ../../iqfiles/isp20
fi
XML_FILES=$(find -type f -name "*.xml")

for file in ${XML_FILES[@]}
do
  XML=$file
  JSON=$(echo ${file/\.xml/\.json})
  echo ">>>>>> Converting $XML to $JSON <<<<<<"
  $BIN $XML $JSON
done

popd
