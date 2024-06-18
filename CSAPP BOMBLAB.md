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
	//分析上述代码可得如下伪代码：
	int[] pasword;
	pasword[0] = 1;
	for(int i = 1; i <= 5; i ++){
		password[i] = password[i - 1] * 2;
	}
	//password: 1 2 4 8 16 32