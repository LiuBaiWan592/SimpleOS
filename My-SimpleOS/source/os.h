/**
 * @FileName    :os.h
 * @Date        :2025-01-08 15:38:38
 * @Author      :LiuBaiWan-Runner
 * @Version     :V1.0.0
 * @Brief       :SimpleOS
 * @Description :公共头文件
 */
#ifndef OS_H
#define OS_H

#define KERNEL_CODE_SEG     ((1 * 8))
#define KERNEL_DATA_SEG     ((2 * 8))
#define APP_CODE_SEG        ((3 * 8) | 3)   // CS Register CPL为3
#define APP_DATA_SEG        ((4 * 8) | 3)   // Segment Selector For Data Segment RPL 为3
#define TASK0_TSS_SEG       ((5 * 8))       // Task-State Segment of Task_0
#define TASK1_TSS_SEG       ((6 * 8))       // Task-State Segment of Task_1
#define SYSCALL_SEG         ((7 * 8))       // 系统调用门
#define TASK0_LDT_SEG       ((8 * 8))       // Task0 LDT 段选择子
#define TASK1_LDT_SEG       ((9 * 8))       // Task1 LDT 段选择子

#define TASK_CODE_SEG       ((0 * 8) | 0x4 | 3)     // LDT 任务代码段
#define TASK_DATA_SEG       ((1 * 8) | 0x4 | 3)     // LDT 任务数据段

#endif // OS_H
