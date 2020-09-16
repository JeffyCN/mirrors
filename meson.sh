echo "# to rm ./build-rga/"
rm -rf build-rga

ORIGINAL_PATH=$PATH
export PATH=/home/lee/new_rk3399_linux/rk3399/buildroot/output/rockchip_rk3399/host/bin:$PATH
#export PATH=/home/lee/rk3399_linux/output/rockchip_rk3399/host/bin:$PATH

export PKG_CONFIG_PATH=/home/lee/new_rk3399_linux/rk3399/buildroot/output/rockchip_rk3399/host/lib/pkgconfig:/home/lee/new_rk3399_linux/rk3399/buildroot/output/rockchip_rk3399/host/usr/aarch64-buildroot-linux-gnu/sysroot/usr/lib/pkgconfig
#export PKG_CONFIG_PATH=/home/lee/rk3399_linux/output/rockchip_rk3399/host/lib/pkgconfig

meson --prefix=/home/lee/new_rk3399_linux/rk3399/external/rga/output build-rga --cross-file cross/cross_file_aarch64.txt

export PATH=$ORIGINAL_PATH

unset PKG_CONFIG_PATH
