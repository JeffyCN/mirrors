   rk_sign_tool is a signing tool for secureboot.it can support 3588|3566|3568|3308|3326|3399|3229|3228h|3368|3228|3288|px30|3328|1808|3228P|1109|1126|2206.
show help of the tool.run it with any parameters.show detail of command,run the command with -h.

step by step:
1.specify chip,just do it once, all of chips are 3588|3566|3568|3308|3326|3399|3229|3228h|3368|3228|3288|px30|3328|1808|3228P|1109|1126|2206
  rk_sign_tool cc --chip 3326
2.generate rsa key pairs ,just do it once.if you have key pairs generated before ,go next
  rk_sign_tool kk --out .
3.load rsa key pairs,just do it onece . if you have key pairs loaded before, go next
  rk_sign_tool lk --key privateKey.pem --pubkey publicKey.pem
4.sign loader
  rk_sign_tool sl --loader loader.bin
5.sign uboot.img trust.img
  rk_sign_tool si --img uboot.img
  rk_sign_tool si --img trust.img
6.sign update.img,it will sign loader,uboot,trust in the update.img
  rk_sign_tool sf --firmware update.img
