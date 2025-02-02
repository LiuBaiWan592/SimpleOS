/**
 * @FileName    :os.c
 * @Date        :2025-01-08 15:39:00
 * @Author      :LiuBaiWan-Runner
 * @Version     :V1.0.0
 * @Brief       :SimpleOS
 * @Description :32位代码，完成多任务的运行
 */
#include "os.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

/**
 * @brief  系统调用处理函数
 * @param  func     功能号
 * @param  str      显示的字符串
 * @param  color    字符串颜色
 * @retval None
 * @note   None
 */
void do_syscall(int func, char *str, char color){
    static int row = 0;

    if(func == 2){
        unsigned short *dest = (unsigned short *)0xb8000 + 80 *row;
        while (*str){
            *dest++ = *str++; (color << 8);
        }
        row = (row >= 25) ? 0 : row + 1;
    }
}

void sys_show(char *str, char color){
    // 远跳转
    uint32_t addr[] = {0, SYSCALL_SEG};    // 偏移，段选择子
    __asm__ __volatile__("lcalll *(%[a])"::[a]"r"(addr));
}

// task_0功能函数
void task_0(void){
    char *str = "task a: 1234";
    uint8_t color = 0;

    for(;;){
        sys_show(str, color++);
    }
}

// task_1功能函数
void task_1(void){
    char *str = "task b: 56789";
    uint8_t color = 0xFF;

    for(;;){
        sys_show(str, color--);
    }
}

#define MAP_ADDR 0x80000000         // 打开分页机制要映射的地址

// 系统页表：将0x0-4MB虚拟地址映射到0-4MB的物理地址，做恒等映射
#define PDE_P (1 << 0)
#define PDE_W (1 << 1)
#define PDE_U (1 << 2)
#define PDE_PS (1 << 7)

// 4KB数组 4KB对齐并初始化
uint8_t map_phy_buffer[4096] __attribute__((aligned(4096))) = {
    0x36};

// 二级页表 4KB对齐并初始化
static uint32_t page_table[1024] __attribute__((aligned(4096))) = {
    PDE_U};

// 页目录表 4KB对齐
uint32_t pg_dir[1024] __attribute__((aligned(4096))) = {
    [0] = (0) | PDE_P | PDE_W | PDE_U | PDE_PS,
};

// 全局描述表（GDT）初始化配置
struct
{
    uint16_t limit_l, base_l, base_m_attr, base_h_limit_h;
} gdt_table[256] __attribute__((aligned(8))) = {
    // 内核代码（CODE）段配置：基地址（Base Address）全0，粒度（G granularity）设置为4KB，段界限（Segment Limit）全1 4KB * 2^20 = 4GB地址空间
    // D/B（Default operation size）为32位段(32-bit segment)，Segment present，the most privileged level DPL=0，code or data，Execute/Read
    [KERNEL_CODE_SEG / 8] = {0xFFFF, 0x0000, 0x9A00, 0x00CF},
    // 内核数据（DATA）段配置：基地址（Base Address）全0，粒度（G granularity）设置为4KB，段界限（Segment Limit）全1 4KB * 2^20 = 4GB地址空间
    // D/B（Default operation size）为32位段(32-bit segment)，Segment present，the most privileged level DPL=0，code or data，Read/Write
    [KERNEL_DATA_SEG / 8] = {0xFFFF, 0x0000, 0x9200, 0x00CF},
    // 应用代码（CODE）段配置：基地址（Base Address）全0，粒度（G granularity）设置为4KB，段界限（Segment Limit）全1 4KB * 2^20 = 4GB地址空间
    // D/B（Default operation size）为32位段(32-bit segment)，Segment present，3 privilege level DPL=3，code or data，Execute/Read
    [APP_CODE_SEG / 8] = {0xFFFF, 0x0000, 0xFA00, 0x00CF},
    // 内核数据（DATA）段配置：基地址（Base Address）全0，粒度（G granularity）设置为4KB，段界限（Segment Limit）全1 4KB * 2^20 = 4GB地址空间
    // D/B（Default operation size）为32位段(32-bit segment)，Segment present，3 privilege level DPL=3，code or data，Read/Write, accessed
    [APP_DATA_SEG / 8] = {0xFFFF, 0x0000, 0xF300, 0x00CF},
    // TSS(Task-State Segment)配置：
    [TASK0_TSS_SEG / 8] = {0x0068, 0x0000, 0xe900, 0x0000},
    [TASK1_TSS_SEG / 8] = {0x0068, 0x0000, 0xe900, 0x0000},
    // 系统调用门配置
    [SYSCALL_SEG / 8] = {0x0000, KERNEL_CODE_SEG, 0xec03, 0x0000},
};

// task0的栈空间
uint32_t task0_dpl0_stack[1024], task0_dpl3_stack[1024], task1_dpl0_stack[1024], task1_dpl3_stack[1024];

// TSS(Task-State Segment)，用于实现中断时上下文切换
uint32_t task0_tss[] = {
    // prelink, esp0, ss0, esp1, ss1, esp2, ss2
    0,  (uint32_t)task0_dpl0_stack + 4*1024, KERNEL_DATA_SEG , /* 后边不用使用 */ 0x0, 0x0, 0x0, 0x0,
    // cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi,
    (uint32_t)pg_dir,  (uint32_t)task_0/*入口地址*/, 0x202, 0xa, 0xc, 0xd, 0xb, (uint32_t)task0_dpl3_stack + 4*1024/* 栈 */, 0x1, 0x2, 0x3,
    // es, cs, ss, ds, fs, gs, ldt, iomap
    APP_DATA_SEG, APP_CODE_SEG, APP_DATA_SEG, APP_DATA_SEG, APP_DATA_SEG, APP_DATA_SEG, 0x0, 0x0,
};

uint32_t task1_tss[] = {
    // prelink, esp0, ss0, esp1, ss1, esp2, ss2
    0,  (uint32_t)task1_dpl0_stack + 4*1024, KERNEL_DATA_SEG , /* 后边不用使用 */ 0x0, 0x0, 0x0, 0x0,
    // cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi,
    (uint32_t)pg_dir,  (uint32_t)task_1/*入口地址*/, 0x202, 0xa, 0xc, 0xd, 0xb, (uint32_t)task1_dpl3_stack + 4*1024/* 栈 */, 0x1, 0x2, 0x3,
    // es, cs, ss, ds, fs, gs, ldt, iomap
    APP_DATA_SEG, APP_CODE_SEG, APP_DATA_SEG, APP_DATA_SEG, APP_DATA_SEG, APP_DATA_SEG, 0x0, 0x0,
};


// 中断描述表（IDT）初始化配置
struct
{
    uint16_t offset_l, selector, attr, offset_h;
} idt_table[256] __attribute__((aligned(8)));

// 封装outb指令:往某一端口写一字节数据
void outb(uint8_t data, uint16_t port)
{
    // outb %al, %dx
    __asm__ __volatile__("outb %[v], %[p]" ::[p] "d"(port), [v] "a"(data));
}

// 任务调度函数，实现两个任务之间进行切换
void task_sched(void){
    static int task_tss = TASK0_TSS_SEG;    // 初始化为刚开始运行Task0

    task_tss = (task_tss == TASK0_TSS_SEG) ? TASK1_TSS_SEG : TASK0_TSS_SEG;     // 切换任务
    // 远跳转
    uint32_t addr[] = {0, task_tss};    // 偏移，段选择子
    __asm__ __volatile__("ljmp *(%[a])"::[a]"r"(addr));
}

// 时钟中断处理函数声明
void timer_int(void);

// 系统调用处理函数声明
void syscall_handler(void);

void os_init(void)
{
    // 初始化8259中断控制器，打开定时器中断
    outb(0x11, 0x20);   // 初始化8259主片
    outb(0x11, 0xA0);   // 初始化8259从片
    outb(0x20, 0x21);   // 写ICW2，配置主片中断向量起始序号为0x20
    outb(0x28, 0xA1);   // 写ICW2，配置从片中断向量起始序号为0x28
    outb(1 << 2, 0x21); // 写ICW3，告诉主片，他的IRQ2端口上级联从片
    outb(2, 0xAA1);     // 写ICW3，告诉从片，他的IRQ2端口连到了主片上
    outb(0x1, 0x21);    // 写ICW4，告诉主片是与8086处理器连接，普通EOI，工作在非缓冲模式
    outb(0x1, 0xA1);    // 写ICW4，告诉从片是与8086处理器连接，普通EOI，工作在非缓冲模式
    outb(0xFE, 0x21);   // 配置主片端口使能，只打开IRQ0，其余中断屏蔽
    outb(0xFF, 0xA1);   // 配置从片端口使能，屏蔽全部中断

    // 初始化8253定时器，每100ms产生一次时钟中断
    int tmo = 1193180 / 100;  // 计算时钟脉冲次数，时钟频率为1193180
    outb(0x36, 0x43);         // 选择定时器通道0，工作在模式3，二进制计数，周期性产生中断
    outb((uint8_t)tmo, 0x40); // 写入低八位到定时器
    outb(tmo >> 8, 0x40);     // 写入高八位到定时器

    // 配置IDT表项，时钟中断对应表项为0x20，使其指向对应的时钟中断处理函数timer_int
    idt_table[0x20].offset_l = (uint32_t)timer_int & 0xFFFF; // 配置低16位的偏移
    idt_table[0x20].offset_h = (uint32_t)timer_int >> 16;    // 配置高16位的偏移
    idt_table[0x20].selector = KERNEL_CODE_SEG;              // 配置选择子为内核代码段
    idt_table[0x20].attr = 0x8E00;                           // 采用中断门（Interrupt Gate）配置 1000 1110

    // 配置TSS(Task-State Segment)的起始地址
    gdt_table[TASK0_TSS_SEG / 8].base_l = (uint16_t)(uint32_t)task0_tss;
    gdt_table[TASK1_TSS_SEG / 8].base_l = (uint16_t)(uint32_t)task1_tss;

    // 配置系统调用门的 Offset in Segment 字段为系统调用处理函数
    gdt_table[SYSCALL_SEG / 8].limit_l = (uint16_t)(uint32_t)syscall_handler;

    // 0x80000000开始的4MB区域的映射
    // 将虚拟地址0x80000000映射到map_phy_buffer所在地址空间
    // 配置页目录表的MAP_ADDR的高十位表项指向已经初始化的页表page_table
    pg_dir[MAP_ADDR >> 22] = (uint32_t)page_table | PDE_P | PDE_W | PDE_U;
    // 配置页表page_table的MAP_ADDR的中间十位表项指向对应数组map_phy_buffer
    page_table[(MAP_ADDR >> 12) & 0x3FF] = (uint32_t)map_phy_buffer | PDE_P | PDE_W | PDE_U;
}
