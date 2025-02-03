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
	.global _start, timer_int, syscall_handler

	// 声明外部外部定义
	.extern os_init

	// 指定以下的代码生成16位的机器指令 这样才能在启动时的实模式下运行
  	.code16

	// 以下是代码区
 	.text
_start: 
	// _start 为0x7c00	
	// x86使用段寄存器中的内容作为内存访问的基址 预先设置好
	// 采用平坦模型 初始化段寄存器都为 0x00  代码段寄存器(cs)默认初始化0x00
	mov $0, %ax		// 不可将数值直接赋给段寄存器 以通用寄存器为桥梁
	mov %ax, %ds	// 设置数据段
	mov %ax, %es	// 设置数据段
	mov %ax, %ss	// 设置栈段
	mov %ax, %gs	// 设置数据段
	mov %ax, %fs	// 设置数据段

	// 初始化栈空间 设置栈的起始地址 压栈由高地址向低地址压栈 栈底为0x7c00
	mov $_start, %esp

// 计算机上电启动后 只取512字节到0x7c00处
// 利用这512字节 将其余部分读取到0x7E00处 即0x7c00的后512字节处
// INT13软中断读取硬盘到内存 重复读取直到成功
read_self_all:
	mov $_start_32, %bx		// 存放到内存的0x7e00地址处 使用 _start_32 代替
	mov $0x2, %cx			// 从硬盘的第二个扇区开始读取 CH:磁道号 CL:起始扇区号
	mov $0x240, %ax			// AH: 0000 0010  0x42 读磁盘命令 AL:0100 0000  0x40 读取64的扇区 每个扇区512B 共32KB
	mov $0x80,  %dx			// DH: 磁头号 DL:驱动器号 0x80 对第一块磁盘进行操作
	int $0x13
	jc read_self_all		// 读取失败，则重复
	
	// 加载idt和gdt表 进入保护模式
	cli						// 关中断
	lgdt [gdt_desc]			// 加载全局描述表(GDT)
	lidt [idt_desc]			// 加载中断描述表(IDT)
	mov $0x1, %eax
	lmsw %ax				// CR0寄存器最低位PE位置1 打开保护模式
	jmp $KERNEL_CODE_SEG, $_start_32	// CS寄存器置为8 CS指向GDT内核代码段 进入32位保护模式代码 跳转到32位指令处运行

	// 跳到引导标志, 由bios在上电后检查
	.org 0x1fe				// 引导标志 510
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

	// 调用os.c中的页表初始化 跳转到c语言中运行
	call os_init

	// 打开分页机制
	mov $pg_dir, %eax		// 配置CR3寄存器
	mov %eax, %cr3
	mov %cr4, %eax			// 配置CR4寄存器
	orl $(1 << 4), %eax		// PSE位 支持4MB分页
	mov %eax, %cr4
	mov %cr0, %eax			// 配置CR0寄存器
	orl $(1 << 31), %eax	// 打开PG位 开启分页机制
	mov %eax, %cr0

	//sti			// 开中断 改为直接操作EFLAGS寄存器
	//jmp .		// 原地跳转 改为通过中断进行跳转

	// 配置TR寄存器 指向当前运行的任务的Task-State Segment
	mov $TASK0_TSS_SEG, %ax
	ltr %ax

	mov $TASK0_LDT_SEG, %ax
	lldt %ax			// 加载任务局部描述表(IDT)

	// 模拟中断返回 利用中断跳转到task_0运行 从而实现从特权级0到特权级3的变化
	// 中断发生时 会自动压入原SS, ESP, EFLAGS, CS, EIP到栈中 手动模拟发生中断时压栈的操作
	push $TASK_DATA_SEG		// 压入SS寄存器 指向应用数据段
	push $task0_dpl3_stack + 1024 * 4	// 压入ESP寄存器 指向应用栈底
	push $0x202 			// 压入EFLAGS寄存器 IF(Interrupt Enable Flag)位置1 开中断
	push $TASK_CODE_SEG		// 压入CS寄存器	指向应用代码段
	push $task_0_entry		// 压入EIP寄存器 指向中断返回后程序执行指令的地址
	iret					// 中断返回 切换至任务0(task_0)

// 时钟中断处理函数
timer_int:
	push %ds			// 中断处理前保护现场 将寄存器值压栈
	pusha				// Push AX, CX, DX, BX, original SP, BP, SI, and DI.

	mov $0x20, %al		// 向主片发送0x20表示中断处理完成
	outb %al, $0x20		// 发送EOI outb %al, %dx

	// 使用内核的数据段寄存器，因为要访问内核的数据
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds
	call task_sched		// 在dpl0中进行任务切换

	popa				// 中断处理后恢复现场 将寄存器值弹出
	pop %ds				// 根据栈的数据结构 先进后出 后进先出
	iret				// 中断返回

syscall_handler:
	push %ds			// 中断处理前保护现场 将寄存器值压栈
	pusha				// Push AX, CX, DX, BX, original SP, BP, SI, and DI.

	// 使用内核的数据段寄存器，因为要访问内核的数据
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds

	mov %esp, %ebp		// 下面压栈时 esp会不断变化 所以使用ebp作为基址
	push 13*4(%ebp)		// 提取出原压入的各项参数 再按相同的顺序压入一遍
	push 12*4(%ebp)
	push 11*4(%ebp)
	call do_syscall
	add $(3*4), %esp
	popa				// 中断处理后恢复现场 将寄存器值弹出
	pop %ds				// 根据栈的数据结构 先进后出 后进先出
	retf $(3*4)

// 进程0入口函数
task_0_entry:
	// 切换到低特权级 重新设置段寄存器和栈空间
	// mov $APP_DATA_SEG, %ax
	mov %ss, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %gs
	mov %ax, %fs
	jmp task_0			// 跳转到c语言task_0进程中运行


// 配置全局描述表寄存器(GDTR)
gdt_desc:
	.word (256 * 8) - 1		// GDT界限 		word:16bit
	.long gdt_table			// GDT起始地址	long:32bit

// 配置中断描述表寄存器(IDTR)
idt_desc:
	.word (256 * 8) - 1		// IDT界限 		word:16bit
	.long idt_table			// IDT起始地址	long:32bit