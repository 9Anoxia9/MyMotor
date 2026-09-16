
list(APPEND TOOLCHAIN_PATH_MAYBE 
    "D:/DevelopTool/gcc-arm-none-eabi-10-2020-q4-major"
)

#在CMAKE寻找编译器时，会让编译器编译一个可执行文件来检测编译器的能力
#但是由于嵌入式开发不会编译成一个可执行文件，因此需要指定为静态库
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#寻找C编译器，进而找到gcc工具路径
if(NOT N32_TOOLCHAIN_PATH)
    if(NOT CMAKE_C_COMPILER)
        find_program(CMAKE_C_COMPILER NAMES arm-none-eabi-gcc HINTS ${TOOLCHAIN_PATH_MAYBE})
        if(NOT CMAKE_C_COMPILER)
            message(FATAL_ERROR "TOOLCAHIN PATH is not found")  
        endif()
    endif()
    #获取${CMAKE_C_COMPILER}执行程序所在目录，一般是bin文件夹
    get_filename_component(N32_TOOLCHAIN_PATH ${CMAKE_C_COMPILER} DIRECTORY)
    #获取bin文件夹的父目录
    get_filename_component(N32_TOOLCHAIN_PATH ${N32_TOOLCHAIN_PATH} DIRECTORY)
endif(NOT N32_TOOLCHAIN_PATH)

if(NOT TARGET_TRIPLET)
    set(TARGET_TRIPLET "arm-none-eabi")
    message(STATUS "TARGET_TRIPLET using: ${TARGET_TRIPLET}")
endif(NOT TARGET_TRIPLET)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

#设置SYSROOT,在编译器翻译时只能用这个路径的资源。这个路径必须是嵌入式资源,也就是arm-none-eabi
#eabi代表嵌入式二进制接口
set(TOOLCHAIN_SYSROOT "${N32_TOOLCHAIN_PATH}/${TARGET_TRIPLET}")
set(TOOLCHAIN_BIN_PATH "${N32_TOOLCHAIN_PATH}/bin")
set(TOOLCHAIN_INC_PATH "${N32_TOOLCHAIN_PATH}/inc")
set(TOOLCHAIN_LIB_PATH "${N32_TOOLCHAIN_PATH}/lib")

set(CMAKE_SYSROOT ${TOOLCHAIN_SYSROOT})

find_program(CMAKE_CXX_COMPILER NAMES ${TARGET_TRIPLET}-g++ HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_ASM_COMPILER NAMES ${TARGET_TRIPLET}-gcc HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_OBJCOPY NAMES ${TARGET_TRIPLET}-objcopy HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_OBJDUMP NAMES ${TARGET_TRIPLET}-objdump HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_SIZE NAMES ${TARGET_TRIPLET}-size HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_DEBUGGER NAMES ${TARGET_TRIPLET}-gdb HINTS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_CPPFILT NAMES ${STM32_TARGET_TRIPLET}-c++filt HINTS ${TOOLCHAIN_BIN_PATH})

function(print_size TARGET)
    add_custom_target(
        print_size_${TARGET}
        ALL COMMAND ${CMAKE_SIZE} "$<TARGET_FILE:${TARGET}>"
        COMMENT "${TARGET} size:"
        DEPENDS ${TARGET}
    )
endfunction(print_size TARGET)

function(generate_file TARGET OUTPUT_EXTENSION OBJCOPY_BFD_OUTPUT)
    get_target_property(TARGET_OUTPUT_NAME ${TARGET} OUTPUT_NAME)
    if(TARGET_OUTPUT_NAME)
        set(OUTPUT_FILE_NAME "${TARGET_OUTPUT_NAME}.${OUTPUT_EXTENSION}")
    else()
        set(OUTPUT_FILE_NAME "${TARGET}.${OUTPUT_EXTENSION}")
    endif(TARGET_OUTPUT_NAME)

    get_target_property(OUTPUT_DIRECTORY ${TARGET} RUNTIME_OUTPUT_DIRECTORY)
    if(OUTPUT_DIRECTORY)
        set(OUTPUT_FILE_PATH "${OUTPUT_DIRECTORY}/${OUTPUT_FILE_NAME}")
    else()
        set(OUTPUT_FILE_PATH "${OUTPUT_FILE_NAME}")
    endif(OUTPUT_DIRECTORY)

    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ${OBJCOPY_BFD_OUTPUT} "$<TARGET_FILE:${TARGET}>"  ${OUTPUT_FILE_PATH}
        COMMENT "Generating ${OBJCOPY_BFD_OUTPUT} file: ${OUTPUT_FILE_NAME}"
    )   
endfunction()

function(generate_ihex_file TARGET)
    generate_file(${TARGET} "hex" "ihex")
endfunction(generate_ihex_file)

function(generate_srec_file)
    generate_file(${TARGET} "srec" "srec")
endfunction(generate_srec_file)

function(generate_bin_file)
    generate_file(${TARGET} "bin" "binary")
endfunction(generate_bin_file)


set(_spec_cond $<OR:$<C_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:GNU>>)

#提供系统调用空壳实现,与语言无关,嵌入式项目应该被链接
if(NOT (TARGET Arm::NoSys))
    add_library(Arm::NoSys INTERFACE IMPORTED)
    target_compile_options(Arm::NoSys INTERFACE $<${_spec_cond}:--specs=nosys.specs>)
    target_link_options(Arm::NoSys INTERFACE $<${_spec_cond}:--specs=nosys.specs>)
endif()

#newlib-nano标准库,C++也需要C库实现,因此不管C还是C++都要链接该库.如果不链接该库,就会链接gcc自带的标准newlib库
if(NOT (TARGET Arm::Nano))
    add_library(Arm::Nano INTERFACE IMPORTED)
    target_compile_options(Arm::Nano INTERFACE $<${_spec_cond}:--specs=nano.specs>)
    target_link_options(Arm::Nano INTERFACE $<${_spec_cond}:--specs=nano.specs>)
endif()

#该接口库的作用是启用newlib-nano标准库中浮点数打印功能
if(NOT (TARGET Arm::Nano::FloatPrint))
    add_library(Arm::Nano::FloatPrint INTERFACE IMPORTED)
    target_link_options(Arm::Nano::FloatPrint INTERFACE
        $<${_spec_cond}:-Wl,--undefined,_printf_float>
    )
endif()

#该接口库的作用是启用newlib-nano标准库中浮点数输入功能
if(NOT (TARGET Arm::Nano::FloatScan))
    add_library(Arm::Nano::FloatScan INTERFACE IMPORTED)
    target_link_options(Arm::Nano::FloatScan INTERFACE
        $<${_spec_cond}:-Wl,--undefined,_scanf_float>
    )
endif()

#C++编译选项
if(NOT (TARGET Arm::CppEmbedded))
    add_library(Arm::CppEmbedded INTERFACE IMPORTED)  
    
    target_link_libraries(Arm::CppEmbedded INTERFACE 
        Arm::NoSys
        Arm::Nano
    )

    target_compile_features(Arm::CppEmbedded INTERFACE cxx_std_17)

    target_compile_options(Arm::CppEmbedded INTERFACE 
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<COMPILE_LANGUAGE:CXX>>:    #只适用于CXX文件
            -fno-rtti                                               #禁止运行时类型识别特性
            -fno-exceptions                                         #禁止CXX异常处理特性
            -fno-threadsafe-statics                                 #禁止局部静态变量初始化线程安全
        >
    )

endif(NOT (TARGET Arm::CppEmbedded))



