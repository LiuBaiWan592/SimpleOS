# SimpleOS

**在Inter X86保护模式下实现的两个进程切换的简单操作系统**

项目源自：[李述铜-用200行代码快速学习x86保护模式编程](https://app7ulykyut1996.pc.xiaoe-tech.com/p/t_pc/course_pc_detail/column/p_63a65325e4b06159f72462ea?product_id=p_63a65325e4b06159f72462ea)、[GitHub](https://github.com/lishutong1024/diy-200lines-os)

## 目录说明

1. image：虚拟磁盘镜像。
2. My SimpleOS：主要系统源代码，增加新功能不断迭代，进行了版本控制。
3. 整体结构框图：项目整体结构图，但是中间实现过程与整体框图略有差异。
4. diy-simpleos-200lines：原项目[源码](https://github.com/lishutong1024/diy-200lines-os)。

## Windows系统下编译工具链配置（Environment）

1. [cmake-3.29.1-windows-x86_64.zip](https://cmake.org/download/)
2. [x86_64-elf-tools-windows13.2.0.zip](https://github.com/lordmilko/i686-elf-tools/releases)
3. [qemu-w32-setup-20170113.exe](https://qemu.weilnetz.de/w64)
4. [git](https://git-scm.com/download/win)的dd工具
5. MinGW-W64 GCC [niXman/mingw-builds-binaries](https://github.com/niXman/mingw-builds-binaries/releases))的mingw32-make.exe工具重命名为make.exe
