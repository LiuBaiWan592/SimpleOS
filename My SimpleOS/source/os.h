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

#endif // OS_H
