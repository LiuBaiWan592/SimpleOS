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
	
	// 进入保护模式
	cli						// 关中断
	lgdt [gdt_desc]			// 加载全局描述表(GDT)
	mov $0x1, %eax
	lmsw %ax				// CR0寄存器最低位PE位置1 打开保护模式
	jmp $KERNEL_CODE_SEG, $_start_32	// CS寄存器置为8 CS指向GDT内核代码段 跳转到32位指令处运行

	.org 0x1fe				// 510
	.byte 0x55, 0xaa		// unsigned char s[] = {0x55, 0xaa}


	// 指定以下的代码生成32位的机器指令 在保护模式下运行
	.code32
	// 以下是代码区
	.text

_start_32:
	// _start_32 为0x7e00	
	// 磁盘读写测试
	//.fill 64*1024, 1, 0x35
	
	// 重新设置段寄存器和栈空间
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov %ax, %gs
	mov %ax, %fs
	mov $_start, %esp
	
	jmp .

gdt_desc:
	.word (256 * 8) - 1		//GDT界限 		word:16bit
	.long gdt_table			//GDT起始地址	long:32bit