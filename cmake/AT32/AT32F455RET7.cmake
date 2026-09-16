#设置SRAM、FLASH大小,用于生成链接脚本
set(SRAM_SIZE  128K)    #大小为128K+16K
set(FLASH_SIZE 512K)
set(STACK_SIZE 12K)
set(HEAP_SIZE  12K)

#设置SRAM、FLASH的起始地址,用于生成链接脚本
set(SRAM_ORIGIN 0x20000000)
set(FLASH_ORIGIN 0x08000000)

#创建一个名为N32::G031的接口库。用于给目标编译时传递编译选项、链接选项。
if(NOT (TARGET AT32::F45::5))
        add_library(AT32::F45::5 INTERFACE IMPORTED)

        #给接口库设置编译器选项，任何链接了该接口库的目标，在编译时都会给编译器传递这些参数
        #-mthumb告诉编译器生成thumb指令，-Wall告诉编译器开启所有常见警告
        #-ffunction-section、-fdata-section告诉编译器给每个函数、变量放入一个独立的段
        #例如 .text.<fun_name> .rodata.<var_name>,在链接脚本中会直接把它们收入.text和.rodata
        target_compile_options(AT32::F45::5 INTERFACE 
                -mthumb
                -mcpu=cortex-m4 
                -mfpu=fpv4-sp-d16       #fpv4-sp表示单精度FPU,d16表示16个双字寄存器
                -mfloat-abi=hard        #启动硬件浮点运算
                -Wall 
                -ffunction-sections 
                -fdata-sections
        )

        #给接口库设置链接器选项，任何链接了该接口库的目标，在链接时将这些参数传递给链接器
        #-Wl,--gc-section告诉编译器，删除没有用到的段
        target_link_options(AT32::F45::5 INTERFACE
                -mthumb 
                -mcpu=cortex-m4
                -mfpu=fpv4-sp-d16       
                -mfloat-abi=hard
                -Wl,--gc-sections       #只是一个链接选项,不是两个！
        )
endif(NOT (TARGET AT32::F45::5))

#编译宏,at32f45x.h里需要芯片名宏来分别定义不同芯片的宏
target_compile_definitions(AT32::F45::5 INTERFACE AT32F455RET7)














