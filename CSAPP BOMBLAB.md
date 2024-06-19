# **Prepartion**
## 实验文件结构：
### bomblab:
	bomb(二进制可执行文件)
	bomb.c
	README
### 实验目的：
insert password of the six bombs;
### 思路：
用gdb看汇编指令来解析答案
### 知识结构：
	C语言
	Linux环境
	gdb常用指令
	汇编语言常用指令
	
----------------------------------------
# **Bomb1**

	gdb bomb
	run
	-----bomb1 boom!
	disas main  //查看源汇编码
	disas phase_1 //查看炸弹1的汇编码
		Dump of assembler code for function phase_1:
		   0x0000000000400ee0 <+0>:     sub    $0x8,%rsp

> [!将答案赋值给esi，一般汇编语言中用esi存储字符串]
> 		   0x0000000000400ee4 <+4>:     mov    $0x402400,%esi

		   0x0000000000400ee9 <+9>:     callq  0x401338 <strings_not_equal>
		   0x0000000000400eee <+14>:    test   %eax,%eax
		   0x0000000000400ef0 <+16>:    je     0x400ef7 <phase_1+23>
		   0x0000000000400ef2 <+18>:    callq  0x40143a <explode_bomb>
		   0x0000000000400ef7 <+23>:    add    $0x8,%rsp
		   0x0000000000400efb <+27>:    retq   
	x/s 0x402400 //解析该地址对应的字符串
		0x402400:       "Border relations with Canada have never been better."
	这样就得到bomb1的破解密码了
---------------------------------
# **Bomb2**

	disas phase_2
		Dump of assembler code for function phase_2:
		   0x0000000000400efc <+0>:     push   %rbp
		   0x0000000000400efd <+1>:     push   %rbx
		   0x0000000000400efe <+2>:     sub    $0x28,%rsp
		   0x0000000000400f02 <+6>:     mov    %rsp,%rsi
		   0x0000000000400f05 <+9>:     callq  0x40145c <read_six_numbers>
		   ==0x0000000000400f0a <+14>:    cmpl   $0x1,(%rsp)== //初始值为1
		   0x0000000000400f0e <+18>:    je     0x400f30 <phase_2+52>
		   0x0000000000400f10 <+20>:    callq  0x40143a <explode_bomb>
		   0x0000000000400f15 <+25>:    jmp    0x400f30 <phase_2+52>
		   0x0000000000400f17 <+27>:    mov    -0x4(%rbx),%eax
		   0x0000000000400f1a <+30>:    add    %eax,%eax
		   0x0000000000400f1c <+32>:    cmp    %eax,(%rbx)
		   0x0000000000400f1e <+34>:    je     0x400f25 <phase_2+41>
		   0x0000000000400f20 <+36>:    callq  0x40143a <explode_bomb>
		   0x0000000000400f25 <+41>:    add    $0x4,%rbx
		   0x0000000000400f29 <+45>:    cmp    %rbp,%rbx
		   ==0x0000000000400f2c <+48>:    jne    0x400f17 <phase_2+27>==  //循环
		   ==0x0000000000400f2e <+50>:    jmp    0x400f3c <phase_2+64>==
		   0x0000000000400f30 <+52>:    lea    0x4(%rsp),%rbx
		   0x0000000000400f35 <+57>:    lea    0x18(%rsp),%rbp
		   0x0000000000400f3a <+62>:    jmp    0x400f17 <phase_2+27>
		   0x0000000000400f3c <+64>:    add    $0x28,%rsp
		   0x0000000000400f40 <+68>:    pop    %rbx
		   0x0000000000400f41 <+69>:    pop    %rbp
		   0x0000000000400f42 <+70>:    retq   
		End of assembler dump.
	disas read_six_numbers //判断是否是6个数
		Dump of assembler code for function read_six_numbers:
		   0x000000000040145c <+0>:     sub    $0x18,%rsp
		   0x0000000000401460 <+4>:     mov    %rsi,%rdx
		   0x0000000000401463 <+7>:     lea    0x4(%rsi),%rcx
		   0x0000000000401467 <+11>:    lea    0x14(%rsi),%rax
		   0x000000000040146b <+15>:    mov    %rax,0x8(%rsp)
		   0x0000000000401470 <+20>:    lea    0x10(%rsi),%rax
		   0x0000000000401474 <+24>:    mov    %rax,(%rsp)
		   0x0000000000401478 <+28>:    lea    0xc(%rsi),%r9
		   0x000000000040147c <+32>:    lea    0x8(%rsi),%r8

> [!x/s 0x4025c3得到密码格式]
> 		   0x0000000000401480 <+36>:    mov    $0x4025c3,%esi

		   0x0000000000401485 <+41>:    mov    $0x0,%eax
		   0x000000000040148a <+46>:    callq  0x400bf0 <__isoc99_sscanf@plt>
		   0x000000000040148f <+51>:    cmp    $0x5,%eax
		   0x0000000000401492 <+54>:    jg     0x401499 <read_six_numbers+61>
		   0x0000000000401494 <+56>:    callq  0x40143a <explode_bomb>
		   0x0000000000401499 <+61>:    add    $0x18,%rsp
		   0x000000000040149d <+65>:    retq   
		End of assembler dump.
	//分析上述代码可得如下伪代码：
	int[] pasword;
	pasword[0] = 1;
	for(int i = 1; i <= 5; i ++){
		password[i] = password[i - 1] * 2;
	}
	//password: 1 2 4 8 16 32
-----------------
# **Bomb3**

disas phase_3

	Dump of assembler code for function phase_3:
		   0x0000000000400f43 <+0>:     sub    $0x18,%rsp
		   0x0000000000400f47 <+4>:     lea    0xc(%rsp),%rcx
		   0x0000000000400f4c <+9>:     lea    0x8(%rsp),%rdx
		   0x0000000000400f51 <+14>:    mov    $0x4025cf,%esi  
		   // "%d %d", 输入两个十进制数
		   0x0000000000400f56 <+19>:    mov    $0x0,%eax //eax：输入字符的个数
		   0x0000000000400f5b <+24>:    callq  0x400bf0 <__isoc99_sscanf@plt>
		   0x0000000000400f60 <+29>:    cmp    $0x1,%eax //输入的字符数 > 1
		   0x0000000000400f63 <+32>:    jg     0x400f6a <phase_3+39>
		   0x0000000000400f65 <+34>:    callq  0x40143a <explode_bomb>
		   0x0000000000400f6a <+39>:    cmpl   $0x7,0x8(%rsp) // 0<= InputNum[0] <= 7
		   0x0000000000400f6f <+44>:    ja     0x400fad <phase_3+106>
		   0x0000000000400f71 <+46>:    mov    0x8(%rsp),%eax
		   0x0000000000400f75 <+50>:    jmpq   *0x402470(,%rax,8)  
		   //switch: case: 1 - 7 + default, case =  0x402470 + 8 * rax
		   //x/8xg 0x402470
			0x402470:       0x0000000000400f7c      0x0000000000400fb9
			0x402480:       0x0000000000400f83      0x0000000000400f8a
			0x402490:       0x0000000000400f91      0x0000000000400f98
			0x4024a0:       0x0000000000400f9f      0x0000000000400fa6 
		    %% jmpq *(xxxx), 表示取了这个地址的数据，也就是说, 
		    $rax为1时取的是数据0x400fb9然后以这个数据为地址跳转 %%
		   0x0000000000400f7c <+57>:    mov    $0xcf,%eax
		   // 用p 0xcf查看对应的值为207
		   //InputNum[0] = 0, InputNum[1] = 207
		   0x0000000000400f81 <+62>:    jmp    0x400fbe <phase_3+123> 
		   0x0000000000400f83 <+64>:    mov    $0x2c3,%eax // 2 707
		   0x0000000000400f88 <+69>:    jmp    0x400fbe <phase_3+123>
		   0x0000000000400f8a <+71>:    mov    $0x100,%eax // 3 256
		   0x0000000000400f8f <+76>:    jmp    0x400fbe <phase_3+123>
		   0x0000000000400f91 <+78>:    mov    $0x185,%eax // 4 389
		   0x0000000000400f96 <+83>:    jmp    0x400fbe <phase_3+123>
		   0x0000000000400f98 <+85>:    mov    $0xce,%eax // 5 206
		   0x0000000000400f9d <+90>:    jmp    0x400fbe <phase_3+123>
		   0x0000000000400f9f <+92>:    mov    $0x2aa,%eax // 6 682
		   0x0000000000400fa4 <+97>:    jmp    0x400fbe <phase_3+123>
		   0x0000000000400fa6 <+99>:    mov    $0x147,%eax // 7 327
		   0x0000000000400fab <+104>:   jmp    0x400fbe <phase_3+123>
		   0x0000000000400fad <+106>:   callq  0x40143a <explode_bomb>
		   0x0000000000400fb2 <+111>:   mov    $0x0,%eax 
		   0x0000000000400fb7 <+116>:   jmp    0x400fbe <phase_3+123>
		   0x0000000000400fb9 <+118>:   mov    $0x137,%eax // 1 311
		   0x0000000000400fbe <+123>:   cmp    0xc(%rsp),%eax
		   0x0000000000400fc2 <+127>:   je     0x400fc9 <phase_3+134>
		   0x0000000000400fc4 <+129>:   callq  0x40143a <explode_bomb>
		   0x0000000000400fc9 <+134>:   add    $0x18,%rsp
		   0x0000000000400fcd <+138>:   retq   
		End of assembler dump.
## Bomb3 password
（以下每个组合任取其一）
	0 207 
	1 311 
	2 707 
	3 256 
	4 389
	5 206
	6 682
	7 327
	
--------------------------------
# **Bomb4**

	disas phase_4  // 0x8(%rsp): InputNum[0], 0xc(%rsp): InputNum[1]
	Dump of assembler code for function phase_4:
	   0x000000000040100c <+0>:     sub    $0x18,%rsp
	   0x0000000000401010 <+4>:     lea    0xc(%rsp),%rcx
	   0x0000000000401015 <+9>:     lea    0x8(%rsp),%rdx
	   0x000000000040101a <+14>:    mov    $0x4025cf,%esi // "%d %d"
	   0x000000000040101f <+19>:    mov    $0x0,%eax
	   0x0000000000401024 <+24>:    callq  0x400bf0 <__isoc99_sscanf@plt>
	   0x0000000000401029 <+29>:    cmp    $0x2,%eax
	   0x000000000040102c <+32>:    jne    0x401035 <phase_4+41>
	   0x000000000040102e <+34>:    cmpl   $0xe,0x8(%rsp) //  x <14
	   0x0000000000401033 <+39>:    jbe    0x40103a <phase_4+46>
	   0x0000000000401035 <+41>:    callq  0x40143a <explode_bomb>
	   0x000000000040103a <+46>:    mov    $0xe,%edx
	   0x000000000040103f <+51>:    mov    $0x0,%esi
	   0x0000000000401044 <+56>:    mov    0x8(%rsp),%edi // func4(InputNum[0])
	   0x0000000000401048 <+60>:    callq  0x400fce <func4>
	   0x000000000040104d <+65>:    test   %eax,%eax // if eax == 0
	   0x000000000040104f <+67>:    jne    0x401058 <phase_4+76>
	   0x0000000000401051 <+69>:    cmpl   $0x0,0xc(%rsp) // if x == 0
	   0x0000000000401056 <+74>:    je     0x40105d <phase_4+81>
	   0x0000000000401058 <+76>:    callq  0x40143a <explode_bomb>
	   0x000000000040105d <+81>:    add    $0x18,%rsp
	   0x0000000000401061 <+85>:    retq   
	End of assembler dump.
	
	disas func4 // from start: edx = 14, esi = 0, edi = InputNum[0]
	Dump of assembler code for function func4: //  x = x >> 1 - 1;
	   0x0000000000400fce <+0>:     sub    $0x8,%rsp
	   0x0000000000400fd2 <+4>:     mov    %edx,%eax // eax = 14
	   0x0000000000400fd4 <+6>:     sub    %esi,%eax // eax = 14
	   0x0000000000400fd6 <+8>:     mov    %eax,%ecx // ecx = 14
	   0x0000000000400fd8 <+10>:    shr    $0x1f,%ecx // ecx = 0, 取符号位
	   0x0000000000400fdb <+13>:    add    %ecx,%eax // eax = 14
	   0x0000000000400fdd <+15>:    sar    %eax // eax = 7
	   0x0000000000400fdf <+17>:    lea    (%rax,%rsi,1),%ecx
	    // ecx = rax = eax = 7, rsi = esi = 0

> [!这里是关键，当且仅当InputNum[0]刚好等于7/3/1/0时，func4返回且eax == 0]
> 	   0x0000000000400fe2 <+20>:    cmp    %edi,%ecx 

	   0x0000000000400fe4 <+22>:    jle    0x400ff2 <func4+36> // InputNum[0] >= ecx
	   0x0000000000400fe6 <+24>:    lea    -0x1(%rcx),%edx
	   0x0000000000400fe9 <+27>:    callq  0x400fce <func4>
	    //假如InputNum[0] <= edx 则继续递归edx = edx >> 1 - 1, 最后 刚好                            InputNum[0]==edx
		    eax = 0;
	   0x0000000000400fee <+32>:    add    %eax,%eax
	   0x0000000000400ff0 <+34>:    jmp    0x401007 <func4+57>
	   0x0000000000400ff2 <+36>:    mov    $0x0,%eax
	   0x0000000000400ff7 <+41>:    cmp    %edi,%ecx 
	   0x0000000000400ff9 <+43>:    jge    0x401007 <func4+57> 
	   // 因为之前在+20那里已经判断过ecx <= InputNum[0], 这里判断 if ecx >= InputNum[0]
	   // 综合起来就是if ecx == InputNum[0], 因此要结束循环InputNum[0]必须刚好是7/3/1/0
	   0x0000000000400ffb <+45>:    lea    0x1(%rcx),%esi // bomb
	   0x0000000000400ffe <+48>:    callq  0x400fce <func4>
	   0x0000000000401003 <+53>:    lea    0x1(%rax,%rax,1),%eaxstop
	   
	   0x0000000000401007 <+57>:    add    $0x8,%rsp
	   0x000000000040100b <+61>:    retq   
	End of assembler dump.
## **Bomb4 Password:**
0 0
1 0
3 0
7 0

---------------
# **Bomb5**

	disas phase_5
	Dump of assembler code for function phase_5:
	   0x0000000000401062 <+0>:     push   %rbx
	   0x0000000000401063 <+1>:     sub    $0x20,%rsp
	   0x0000000000401067 <+5>:     mov    %rdi,%rbx  // rdi: 输入的字符串
	   0x000000000040106a <+8>:     mov    %fs:0x28,%rax
	   0x0000000000401073 <+17>:    mov    %rax,0x18(%rsp)
	   0x0000000000401078 <+22>:    xor    %eax,%eax
	   0x000000000040107a <+24>:    callq  0x40131b <string_length>
	   0x000000000040107f <+29>:    cmp    $0x6,%eax // 密码为6个字符的字符串
	   0x0000000000401082 <+32>:    je     0x4010d2 <phase_5+112>
	   0x0000000000401084 <+34>:    callq  0x40143a <explode_bomb>
	   0x0000000000401089 <+39>:    jmp    0x4010d2 <phase_5+112>
	   //for循环 for(int i = 0 ; i < 6 ; i ++)
			   {
				   edx = InputString[i] & 0xf;
				   rsi = 0x4024b0[edx];
			   }
		   0x000000000040108b <+41>:    movzbl (%rbx,%rax,1),%ecx  
		   // ecx: 依次遍历取输入的6个字符, eg..: i = 0, ecx = 'a'
		   0x000000000040108f <+45>:    mov    %cl,(%rsp) // cl = ecx低8位 = 'a'
		   0x0000000000401092 <+48>:    mov    (%rsp),%rdx
		   0x0000000000401096 <+52>:    and    $0xf,%edx //取得0x4024b0的位数rdx
		   0x0000000000401099 <+55>:    movzbl 0x4024b0(%rdx),%edx
		   0x00000000004010a0 <+62>:    mov    %dl,0x10(%rsp,%rax,1)
		   0x00000000004010a4 <+66>:    add    $0x1,%rax
		   0x00000000004010a8 <+70>:    cmp    $0x6,%rax
		   0x00000000004010ac <+74>:    jne    0x40108b <phase_5+41>
		   // x/s 0x4024b0
		   //0x4024b0 <array.3449>:  "maduiersnfotvbylSo you think you can stop the 
		   //bomb with ctrl-c, do you?" , 在0x4024b0 + rax
		   //找到第一位'f'对应位数为9, 即InputString[0]应在与0xf and 后得到0x9，以此类推
		   // 因此, InputString[0]- 'f' - rax = 9 - 最后四位应为1001 - 'i' etc..
		   //InputString[1] - 'l' - rax = 15 - 最后四位应为1111 - 'o' etc..
		   //InputString[2] - 'y' - rax = 14 - 最后四位应为1110 - 'n' etc..
		   //InputString[3] - 'e' - rax = 5 - 最后四位应为0101 - 'e' etc..
		   //InputString[4] - 'r' - rax = 6 - 最后四位应为0110 - 'f' etc..
		   //InputString[5] - 's' - rax = 7 - 最后四位应为0111 - 'g' etc..
		   //password 1 : ionefg etc..
	   0x00000000004010ae <+76>:    movb   $0x0,0x16(%rsp)
	   0x00000000004010b3 <+81>:    mov    $0x40245e,%esi 
	   // esi，一般用来存储字符串，x/s 0x40245e 得到变形后的答案"flyers"
	   //因此根据上面的for循环，逆向还原输入应为""
	   0x00000000004010b8 <+86>:    lea    0x10(%rsp),%rdi 
	   // "flyers"应与0x10(%rsp)+ 1 to 5 对应
	   0x00000000004010bd <+91>:    callq  0x401338 <strings_not_equal>
	   0x00000000004010c2 <+96>:    test   %eax,%eax
	   0x00000000004010c4 <+98>:    je     0x4010d9 <phase_5+119>
	   0x00000000004010c6 <+100>:   callq  0x40143a <explode_bomb>
	   0x00000000004010cb <+105>:   nopl   0x0(%rax,%rax,1)
	   0x00000000004010d0 <+110>:   jmp    0x4010d9 <phase_5+119>
	   0x00000000004010d2 <+112>:   mov    $0x0,%eax
	   0x00000000004010d7 <+117>:   jmp    0x40108b <phase_5+41>
	   0x00000000004010d9 <+119>:   mov    0x18(%rsp),%rax
	   0x00000000004010de <+124>:   xor    %fs:0x28,%rax
	   0x00000000004010e7 <+133>:   je     0x4010ee <phase_5+140>
	   0x00000000004010e9 <+135>:   callq  0x400b30 <__stack_chk_fail@plt>
	   0x00000000004010ee <+140>:   add    $0x20,%rsp
	   0x00000000004010f2 <+144>:   pop    %rbx
	   0x00000000004010f3 <+145>:   retq   
	End of assembler dump.
## **Bomb5 password:**
InputString[0] - 最后四位应为1001 - 'i' etc..
InputString[1] - 最后四位应为1111 - 'o' etc..
InputString[2] - 最后四位应为1110 - 'n' etc..
InputString[3] - 最后四位应为0101 - 'e' etc..
InputString[4] - 最后四位应为0110 - 'f' etc..
InputString[5] - 最后四位应为0111 - 'g' etc..

-----------------
# **Bomb6**

	disas phase_6
	Dump of assembler code for function phase_6:
	   0x00000000004010f4 <+0>:     push   %r14
	   0x00000000004010f6 <+2>:     push   %r13
	   0x00000000004010f8 <+4>:     push   %r12
	   0x00000000004010fa <+6>:     push   %rbp
	   0x00000000004010fb <+7>:     push   %rbx
	   0x00000000004010fc <+8>:     sub    $0x50,%rsp
	   0x0000000000401100 <+12>:    mov    %rsp,%r13
	   0x0000000000401103 <+15>:    mov    %rsp,%rsi
	   0x0000000000401106 <+18>:    callq  0x40145c <read_six_numbers> 
	   // password -> six numbers
	   0x000000000040110b <+23>:    mov    %rsp,%r14
	   0x000000000040110e <+26>:    mov    $0x0,%r12d
	   
		   // from start: eax = InputNum[0], r12d = 0, r13 = InputNum[0]
		   0x0000000000401114 <+32>:    mov    %r13,%rbp
		   0x0000000000401117 <+35>:    mov    0x0(%r13),%eax 
		   0x000000000040111b <+39>:    sub    $0x1,%eax
		   0x000000000040111e <+42>:    cmp    $0x5,%eax //InputNum[i] - 1 <= 5
		   0x0000000000401121 <+45>:    jbe    0x401128 <phase_6+52>
		   0x0000000000401123 <+47>:    callq  0x40143a <explode_bomb>
		   0x0000000000401128 <+52>:    add    $0x1,%r12d
		   0x000000000040112c <+56>:    cmp    $0x6,%r12d
		   0x0000000000401130 <+60>:    je     0x401153 <phase_6+95>
		   0x0000000000401132 <+62>:    mov    %r12d,%ebx
		   0x0000000000401135 <+65>:    movslq %ebx,%rax
		   0x0000000000401138 <+68>:    mov    (%rsp,%rax,4),%eax 
		   //(%rsp,%rax,4) = InputNum[i + 1]
		   0x000000000040113b <+71>:    cmp    %eax,0x0(%rbp)
		   0x000000000040113e <+74>:    jne    0x401145 <phase_6+81>
		   0x0000000000401140 <+76>:    callq  0x40143a <explode_bomb>
		   0x0000000000401145 <+81>:    add    $0x1,%ebx
		   0x0000000000401148 <+84>:    cmp    $0x5,%ebx
		   0x000000000040114b <+87>:    jle    0x401135 <phase_6+65>
		   0x000000000040114d <+89>:    add    $0x4,%r13 // r13 = InputNum[i ++]
		   0x0000000000401151 <+93>:    jmp    0x401114 <phase_6+32>
		   ## for(int i = 0; i < 6 ; i ++)
		   {
			   if(InputNum[i] > 6 || InputNum[i] <= 0) return expolode_bomb;
			   for(int j = i + 1; j < 6; j ++)
			   {
				   if(InputNum[j] == InputNum[i]) return explode_bomb;
			   }
		   }##
	   0x0000000000401153 <+95>:    lea    0x18(%rsp),%rsi
	   0x0000000000401158 <+100>:   mov    %r14,%rax
	   0x000000000040115b <+103>:   mov    $0x7,%ecx
	      //from start: rsi = rsp + 0x18 , rax = rsp = InputNum[0], ecx = 7;
		   0x0000000000401160 <+108>:   mov    %ecx,%edx
		   0x0000000000401162 <+110>:   sub    (%rax),%edx
		   0x0000000000401164 <+112>:   mov    %edx,(%rax) 
		   // InputNum[i] = 7 - InputNum[i]
		   0x0000000000401166 <+114>:   add    $0x4,%rax //rax += 4;
		   0x000000000040116a <+118>:   cmp    %rsi,%rax
		   0x000000000040116d <+121>:   jne    0x401160 <phase_6+108>
		   ## for(int i = 0 ; i < 6; i ++)
		   {
			   InputNum[i] = 7 - InputNum[i];
		   }##
	   0x000000000040116f <+123>:   mov    $0x0,%esi
	   0x0000000000401174 <+128>:   jmp    0x401197 <phase_6+163>
	   0x0000000000401176 <+130>:   mov    0x8(%rdx),%rdx
	   0x000000000040117a <+134>:   add    $0x1,%eax
	   0x000000000040117d <+137>:   cmp    %ecx,%eax
	   0x000000000040117f <+139>:   jne    0x401176 <phase_6+130> 
	   // from 130 to 139: eax ++ to ecx(InputNum[0]), rdx ++ to node(0 +   
	   InputNum[0]);
	   0x0000000000401181 <+141>:   jmp    0x401188 <phase_6+148>
	   0x0000000000401183 <+143>:   mov    $0x6032d0,%edx
		   ## x/30 0x6032d0
			0x6032d0 <node1>:       "L\001" // "L\001" (int)-> 332
			0x6032d3 <node1+3>:     ""
			0x6032d4 <node1+4>:     "\001" // (int)-> InputNum[0]
			0x6032d6 <node1+6>:     ""
			0x6032d7 <node1+7>:     ""
			0x6032d8 <node1+8>:     "\340\062`" //(int)-> 0x6032e0 = node2 
			0x6032dc <node1+12>:    ""
			0x6032dd <node1+13>:    ""
			0x6032de <node1+14>:    ""
			0x6032df <node1+15>:    ""
			0x6032e0 <node2>:       "\250"
			0x6032e2 <node2+2>:     ""
			0x6032e3 <node2+3>:     ""
			0x6032e4 <node2+4>:     "\002"
			0x6032e6 <node2+6>:     ""
			0x6032e7 <node2+7>:     ""
			0x6032e8 <node2+8>:     "\360\062`"
			0x6032ec <node2+12>:    ""
			0x6032ed <node2+13>:    ""
			0x6032ee <node2+14>:    ""
			0x6032ef <node2+15>:    ""
			0x6032f0 <node3>:       "\234\003"
			0x6032f3 <node3+3>:     ""
			0x6032f4 <node3+4>:     "\003"
			0x6032f6 <node3+6>:     ""
			0x6032f7 <node3+7>:     ""
			0x6032f8 <node3+8>:     ""
			0x6032f9 <node3+9>:     "3`"
			0x6032fc <node3+12>:    ""
			0x6032fd <node3+13>:    ""
			(gdb) x/64 0x6032d0
			0x6032d0 <node1>:       "L\001"
			0x6032d3 <node1+3>:     ""
			0x6032d4 <node1+4>:     "\001"
			0x6032d6 <node1+6>:     ""
			0x6032d7 <node1+7>:     ""
			0x6032d8 <node1+8>:     "\340\062`"
			0x6032dc <node1+12>:    ""
			0x6032dd <node1+13>:    ""
			0x6032de <node1+14>:    ""
			0x6032df <node1+15>:    ""
			0x6032e0 <node2>:       "\250"
			0x6032e2 <node2+2>:     ""
			0x6032e3 <node2+3>:     ""
			0x6032e4 <node2+4>:     "\002"
			0x6032e6 <node2+6>:     ""
			0x6032e7 <node2+7>:     ""
			0x6032e8 <node2+8>:     "\360\062`"
			0x6032ec <node2+12>:    ""
			0x6032ed <node2+13>:    ""
			0x6032ee <node2+14>:    ""
			0x6032ef <node2+15>:    ""
			0x6032f0 <node3>:       "\234\003"
			0x6032f3 <node3+3>:     ""
			0x6032f4 <node3+4>:     "\003"
			0x6032f6 <node3+6>:     ""
			0x6032f7 <node3+7>:     ""
			0x6032f8 <node3+8>:     ""
			0x6032f9 <node3+9>:     "3`"
			0x6032fc <node3+12>:    ""
			0x6032fd <node3+13>:    ""
			0x6032fe <node3+14>:    ""
			0x6032ff <node3+15>:    ""
			0x603300 <node4>:       "\263\002"
			0x603303 <node4+3>:     ""
			0x603304 <node4+4>:     "\004"
			0x603306 <node4+6>:     ""
			0x603307 <node4+7>:     ""
			0x603308 <node4+8>:     "\020\063`"
			0x60330c <node4+12>:    ""
			0x60330d <node4+13>:    ""
			0x60330e <node4+14>:    ""
			0x60330f <node4+15>:    ""
			0x603310 <node5>:       "\335\001"
			0x603313 <node5+3>:     ""
			0x603314 <node5+4>:     "\005"
			0x603316 <node5+6>:     ""
			0x603317 <node5+7>:     ""
			0x603318 <node5+8>:     " 3`"
			0x60331c <node5+12>:    ""
			0x60331d <node5+13>:    ""
			0x60331e <node5+14>:    ""
			0x60331f <node5+15>:    ""
			0x603320 <node6>:       "\273\001"
			0x603323 <node6+3>:     ""
			0x603324 <node6+4>:     "\006"
			0x603326 <node6+6>:     ""
			0x603327 <node6+7>:     ""
			0x603328 <node6+8>:     ""
			0x603329 <node6+9>:     ""
			0x60332a <node6+10>:    ""
			0x60332b <node6+11>:    ""
			0x60332c <node6+12>:    ""
			0x60332d <node6+13>:    ""
			0x60332e <node6+14>:    ""
		   ## 
		   node{
			   int nodeval; 
			   int val; 
			   node* next;
		   }
		   node1 = {332, InputNum[0], node->next} 
		   node2 = {168, InputNum[1], node->next} 
		   node3 = {924, InputNum[2], node->next} 
		   node4 = {691, InputNum[3], node->next} 
		   node5 = {477, InputNum[4], node->next} 
		   node6 = {443, InputNum[5], node->next} 
		   ##
	   0x0000000000401188 <+148>:   mov    %rdx,0x20(%rsp,%rsi,2) 
	   //rsp + 20 = 0x6032d0
	   0x000000000040118d <+153>:   add    $0x4,%rsi
	   0x0000000000401191 <+157>:   cmp    $0x18,%rsi
	   0x0000000000401195 <+161>:   je     0x4011ab <phase_6+183>
	   0x0000000000401197 <+163>:   mov    (%rsp,%rsi,1),%ecx  
	   ////from+148 to +163:  node[i] switch with node[7-i];
	   0x000000000040119a <+166>:   cmp    $0x1,%ecx
	   0x000000000040119d <+169>:   jle    0x401183 <phase_6+143>
	   0x000000000040119f <+171>:   mov    $0x1,%eax
	   0x00000000004011a4 <+176>:   mov    $0x6032d0,%edx
	   0x00000000004011a9 <+181>:   jmp    0x401176 <phase_6+130>
	   --
	   0x00000000004011ab <+183>:   mov    0x20(%rsp),%rbx
	   0x00000000004011b0 <+188>:   lea    0x28(%rsp),%rax
	   0x00000000004011b5 <+193>:   lea    0x50(%rsp),%rsi
		   0x00000000004011ba <+198>:   mov    %rbx,%rcx
		   0x00000000004011bd <+201>:   mov    (%rax),%rdx
		   0x00000000004011c0 <+204>:   mov    %rdx,0x8(%rcx)
		   0x00000000004011c4 <+208>:   add    $0x8,%rax
		   0x00000000004011c8 <+212>:   cmp    %rsi,%rax
		   0x00000000004011cb <+215>:   je     0x4011d2 <phase_6+222>
		   0x00000000004011cd <+217>:   mov    %rdx,%rcx
		   0x00000000004011d0 <+220>:   jmp    0x4011bd <phase_6+201>
	    183 to 220:
		   ## for(int i = 5; i >= 1 ; i --)
			   {
				node[i].next = node[i - 1]; 
				// 从内存上1-6重新赋值next链表(因为前面148-163已经把顺序打乱了)
			   }
			##
	   0x00000000004011d2 <+222>:   movq   $0x0,0x8(%rdx)
	   0x00000000004011da <+230>:   mov    $0x5,%ebp
	   0x00000000004011df <+235>:   mov    0x8(%rbx),%rax
	   0x00000000004011e3 <+239>:   mov    (%rax),%eax
	   0x00000000004011e5 <+241>:   cmp    %eax,(%rbx)
	   0x00000000004011e7 <+243>:   jge    0x4011ee <phase_6+250> 
	   0x00000000004011e9 <+245>:   callq  0x40143a <explode_bomb>
	   0x00000000004011ee <+250>:   mov    0x8(%rbx),%rbx
	   0x00000000004011f2 <+254>:   sub    $0x1,%ebp
	   0x00000000004011f5 <+257>:   jne    0x4011df <phase_6+235>
	   // for 此时内存上的node1-6对应的值应该是单调递减的，否则就炸
	   //node1->nodeval > node2->nodeval > node3->nodeval > node4->nodeval > node5
	   ->nodeval > node6->nodeval 
	   ##  the node order is:
		   before:
		   0x6032d0 node1 = {332, InputNum[0], node->next} 
		   0x6032e0 node2 = {168, InputNum[1], node->next} 
		   0x6033f0 node3 = {924, InputNum[2], node->next} 
		   0x603300 node4 = {691, InputNum[3], node->next} 
		   0x603310 node5 = {477, InputNum[4], node->next} 
		   0x603320 node6 = {443, InputNum[5], node->next} 
		   exchange: node(i) switch with node(7 - InputNum[i])
		   now:
		   node1 = {924, InputNum[0], node->next} 
		   node2 = {691, InputNum[1], node->next} 
		   node3 = {477, InputNum[2], node->next} 
		   node4 = {443, InputNum[3], node->next} 
		   node5 = {332, InputNum[4], node->next} 
		   node6 = {168, InputNum[5], node->next} 
		   therefore password is:
		   4 3 2 1 6 5
		##
	   0x00000000004011f7 <+259>:   add    $0x50,%rsp
	   0x00000000004011fb <+263>:   pop    %rbx
	   0x00000000004011fc <+264>:   pop    %rbp
	   0x00000000004011fd <+265>:   pop    %r12
	   0x00000000004011ff <+267>:   pop    %r13
	   0x0000000000401201 <+269>:   pop    %r14
	   0x0000000000401203 <+271>:   retq   
	End of assembler dump.
## Bomb6 password:
4 3 2 1 6 5