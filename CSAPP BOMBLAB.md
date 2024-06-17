**Prepartion**
实验文件结构：
bomblab:
	bomb(二进制可执行文件)
	bomb.c
	README
实验目的：insert password of the six bombs;
思路：初始不知道密码可以输入空然后用gdb看汇编指令和机器码来解析答案
知识结构：
	C语言
	Linux环境
	gdb常用指令
	汇编语言常用指令
**Bomb1**
	gdb bomb
	run
	-----bomb1 boom!
	disas main  //查看源汇编码
	disas phase_1 //查看炸弹1的汇编码
		Dump of assembler code for function phase_1:
		   0x0000000000400ee0 <+0>:     sub    $0x8,%rsp
		   ==0x0000000000400ee4 <+4>:     mov    $0x402400,%esi==
		   0x0000000000400ee9 <+9>:     callq  0x401338 <strings_not_equal>
		   0x0000000000400eee <+14>:    test   %eax,%eax
		   0x0000000000400ef0 <+16>:    je     0x400ef7 <phase_1+23>
		   0x0000000000400ef2 <+18>:    callq  0x40143a <explode_bomb>
		   0x0000000000400ef7 <+23>:    add    $0x8,%rsp
		   0x0000000000400efb <+27>:    retq   
	x/s 0x402400 //解析该地址对应的字符串
		0x402400:       "Border relations with Canada have never been better."
	这样就得到bomb1的破解密码了
