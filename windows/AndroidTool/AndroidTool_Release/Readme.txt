v2.48更新内容:
1.支持sparse镜像烧录,且擦除分区尾部的verity信息
2.支持gpt分区表烧录,可以是partition_table格式的gpt文件或者parameter文件
3.支持mtp,uvc设备的切换

v2.5更新内容:
1.解决nand擦除flash问题

v2.52更新内容：
1.gpt支持固件uuid
通过在parameter文件中，按以下格式指定uuid:
uuid:分区名=00000000-0000-0000-0000-000000000000

v2.56更新内容：
1.fix不支持gpt错误

v2.57更新内容:
1.烧录前检查paramter是否有被修改和自动更新

v2.58更新内容:
1.fix "wait for maskrom failed" bug

v2.59:
1.sync with v1.15 upgradelib for fix sparse download bug

v2.61:
1.add select and unselect all of download items

v2.62:
1.support eraselba
2.modify procedure of download progress show

v2.63:
1.support no reset when only download loader
2.support rk gpt parsing

v2.64:
1.fix upgrade update.img with more than 16 items

v2.65:
1.add automatic configuration after close the program

v2.66:
1.fix testdevice bug when device is lowerformat

v2.67:
handle fill chunk 0 case when download sparse
