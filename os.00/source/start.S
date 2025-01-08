 /**
  * @FileName    :start.S
  * @Date        :2025-01-08 15:38:05
  * @Author      :LiuBaiWan-Runner
  * @Version     :V1.0.0
  * @Brief       :SimpleOS
  * @Description :16位与32位的启动混合代码
  */
	#include "os.h"

	// 声明本地以下符号是全局的，在其它源文件中可以访问
	.global _start

	// 指定以下的代码生成16位的机器指令 这样才能在启动时的实模式下运行
  	.code16

	// 以下是代码区
 	.text
_start: 
	// _start 为0x7c00	
	// 初始化段寄存器都为 0x00  代码段寄存器(cs)默认初始化0x00
	mov $0, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov %ax, %gs
	mov %ax, %fs

	// 初始化栈空间 压栈由高地址向低地址压栈 栈底为0x7c00
	mov $_start, %esp
	
	// INT13软中断读取硬盘到内存 重复读取直到成功
read_self_all:
	mov $_start_32, %bx		// 存放到内存的0x7e00地址处 使用 _start_32 代替
	mov $0x2, %cx			// 从硬盘的第二个扇区开始读取
	mov $0x240, %ax			// AL:0100 0000 读取64的扇区 每个扇区512B 共32KB AH: 0000 0010 读磁盘
	mov $0x80,  %dx			// 对第一块磁盘进行操作
	int $0x13
	jc read_self_all
	
	jmp .

	.org 0x1fe			// 510
	.byte 0x55, 0xaa		// unsigned char s[] = {0x55, 0xaa}

_start_32:
	// _start_32 为0x7e00	
	// 磁盘读写测试
	.fill 64*1024, 1, 0x35