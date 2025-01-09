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

#define PDE_P (1 << 0)
#define PDE_W (1 << 1)
#define PDE_U (1 << 2)
#define PDE_PS (1 << 7)

// 页目录表 4KB对齐
uint32_t pg_dir[1024] __attribute__((aligned(4096))) = {
    [0] = (0) | PDE_P | PDE_W | PDE_U | PDE_PS,};

// 全局描述表（GDT）初始化配置
struct
{
    uint16_t limit_l, base_l, base_m_attr, base_h_limit_h;
} gdt_table[256] __attribute__((aligned(8))) = {
    // 内核代码（CODE）段配置：基地址（Base Address）全0，粒度（G granularity）设置为4KB，段界限（Segment Limit）全1 4KB * 2^20 = 4GB地址空间
    // D/B（Default operation size）为32位段(32-bit segment)，Segment present，the most privileged level，code or data，Execute/Read
    [KERNEL_CODE_SEG / 8] = {0xFFFF, 0x0000, 0x9A00, 0x00CF},
    // 内核数据（DATA）段配置：基地址（Base Address）全0，粒度（G granularity）设置为4KB，段界限（Segment Limit）全1 4KB * 2^20 = 4GB地址空间
    // D/B（Default operation size）为32位段(32-bit segment)，Segment present，the most privileged level，code or data，Read/Write
    [KERNEL_DATA_SEG / 8] = {0xFFFF, 0x0000, 0x9200, 0x00CF}};
