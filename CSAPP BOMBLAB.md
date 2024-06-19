**Prepartion**
实验文件结构：
bomblab:
	bomb(二进制可执行文件)
	bomb.c
	README
实验目的：insert password of the six bombs;
思路：用gdb看汇编指令来解析答案
知识结构：
	C语言
	Linux环境
	gdb常用指令
	汇编语言常用指令
	
----------------------------------------
**Bomb1**

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
**Bomb2**

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
**Bomb3**

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
因此**Bomb3 password:（以下每个组合任取其一）** 
	0 207 
	1 311 
	2 707 
	3 256 
	4 389
	5 206
	6 682
	7 327
	
--------------------------------
**Bomb4**

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
**Bomb4 Password:**
0 0
1 0
3 0
7 0

---------------
**Bomb5**

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
**Bomb5 password:**
InputString[0] - 最后四位应为1001 - 'i' etc..
InputString[1] - 最后四位应为1111 - 'o' etc..
InputString[2] - 最后四位应为1110 - 'n' etc..
InputString[3] - 最后四位应为0101 - 'e' etc..
InputString[4] - 最后四位应为0110 - 'f' etc..
InputString[5] - 最后四位应为0111 - 'g' etc..