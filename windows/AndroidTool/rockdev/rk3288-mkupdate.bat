mklink /J Image ..\..\..\..\rockdev
Afptool -pack ./ Image\update.img

RKImageMaker.exe -RK320A Image\MiniLoaderAll.bin  Image\update.img update.img -os_type:androidos

rem update.img is new format, Image\update.img is old format, so delete older format
del  Image\update.img

pause 
