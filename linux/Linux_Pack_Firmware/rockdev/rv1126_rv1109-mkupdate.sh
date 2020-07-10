#!/bin/bash
pause()
{
echo "Press any key to quit:"
read -n1 -s key
exit 1
}
echo "start to make update.img..."
if [ ! -f "Image/parameter" -a ! -f "Image/parameter.txt" ]; then
	echo "Error:No found parameter!"
	exit 1
fi
if [ ! -f "package-file" ]; then
	echo "Error:No found package-file!"
	exit 1
fi
./afptool -pack ./ Image/update.img || pause

# RK_CHIP define on device/rockchip/rv1126_rv1109/BoardConfig*.mk
case $RK_CHIP in
	RV1109)
		chip_name="-RK1109"
		;;
	RV1126 | *)
		chip_name="-RK1126"
		;;
esac
./rkImageMaker $chip_name Image/MiniLoaderAll.bin Image/update.img update.img -os_type:androidos || pause
echo "Making $RK_CHIP update.img OK."
#echo "Press any key to quit:"
#read -n1 -s key
exit $?
