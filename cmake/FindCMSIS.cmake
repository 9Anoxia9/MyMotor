#寻找需要的CMSIS头文件目录
find_path(CMSIS_CORE_INC_PATH
    NAMES cmsis_gcc.h
    HINTS "${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/cmsis/cm4/core_support"
    NO_DEFAULT_PATHs
)

if(NOT TARGET CMSIS::CORE)
    add_library(CMSIS::CORE INTERFACE IMPORTED)
    target_include_directories(CMSIS::CORE INTERFACE ${CMSIS_CORE_INC_PATH})
endif()

find_path(CMSIS_DEVICE_INC_PATH
    NAMES at32f45x.h
    HINTS "${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/cmsis/cm4/device_support"
    NO_DEFAULT_PATH
)

#寻找需要的CMSIS源文件
find_file(CMSIS_STARTUP_SRC
        NAMES startup_at32f45x.s
        PATHS "${CMSIS_DEVICE_INC_PATH}/startup"
        NO_DEFAULT_PATH
)
message("FindCMSIS.cmake 28 lines: startup file path:${CMSIS_STARTUP_SRC}")

find_file(CMSIS_SYSTEMINIT_SRC
        NAMES system_at32f45x.c
        PATHS "${CMSIS_DEVICE_INC_PATH}"
        NO_DEFAULT_PATH
)

list(APPEND CMSIS_SRC 
    "${CMSIS_STARTUP_SRC}"
    "${CMSIS_SYSTEMINIT_SRC}"
)

if(NOT TARGET CMSIS::DEVICE)

    add_library(CMSIS::DEVICE INTERFACE IMPORTED)
    target_include_directories(CMSIS::DEVICE INTERFACE 
            ${CMSIS_DEVICE_INC_PATH}
            ${CMSIS_DEVICE_INC_PATH}/../../../driver/inc

    )
    target_sources(CMSIS::DEVICE INTERFACE ${CMSIS_SRC})

    target_link_libraries(CMSIS::DEVICE INTERFACE 
                CMSIS::CORE 
                AT32::F45::5
    )

endif()

include(AT32/AT32F455RET7)

set(OUTPUT_LD_FILE "${CMAKE_CURRENT_BINARY_DIR}/AT32F455RET7.ld")

#定义一个规则，cmake不会自动执行该规则，只有有文件依赖该规则生成的文件
#时，才会执行该规则生成文件
add_custom_command(
    OUTPUT "${OUTPUT_LD_FILE}"
    COMMAND ${CMAKE_COMMAND} 
        -DFLASH_SIZE=${FLASH_SIZE}
        -DRAM_SIZE=${SRAM_SIZE}
        -DSTACK_SIZE=${STACK_SIZE}
        -DHEAP_SIZE=${HEAP_SIZE}
        -DFLASH_ORIGIN=${FLASH_ORIGIN}
        -DRAM_ORIGIN=${SRAM_ORIGIN}
        -DLINKER_SCRIPT=${OUTPUT_LD_FILE}
        -P "${CMAKE_CURRENT_LIST_FILE}/../AT32/linker_ld.cmake"
)
#将生成链接脚本的逻辑封装成一个伪目标，然后让接口库依赖于该伪目标。任何链接
#依赖该接口库的编译文件在编译时必须确保链接文件已经生成
add_custom_target(CMSIS_LD_FILE DEPENDS "${OUTPUT_LD_FILE}")
add_dependencies(CMSIS::DEVICE INTERFACE CMSIS_LD_FILE)

#在链接时，告诉链接器强制使用该链接脚本设置的内存布局
target_link_options(CMSIS::DEVICE INTERFACE -T "${OUTPUT_LD_FILE}")

get_target_property(TARGET_TYPE CMSIS::DEVICE TYPE)
if(TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
    set(INTERFACE_PREFIX "INTERFACE_")
endif()

#获取当前目标的接口链接依赖项
get_target_property(LINK_DEPENDS CMSIS::DEVICE ${INTERFACE_PREFIX}LINK_DEPENDS)
if(LINK_DEPENDS STREQUAL "LINK_DEPENDS-NOTFOUND")
    set(LINK_DEPENDS "")
endif()

list(APPEND LINK_DEPENDS "${OUTPUT_LD_FILE}")


set_target_properties(CMSIS::DEVICE PROPERTIES ${INTERFACE_PREFIX}LINK_DEPENDS "${LINK_DEPENDS}")

message(STATUS "CMSIS Libraries:")
message("                       CMSIS::CORE")
message("                       CMSIS::DEVICE")













