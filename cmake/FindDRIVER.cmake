find_path(DRIVER_SRC_PATH
    NAMES  at32f45x_acc.c
    HINTS "${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/driver/src" 
    NO_DEFAULT_PATHs
)


#获取外设列表，最终output_list存储的是诸如：adc、beeper、i2c....
function(generate_driver_list output_list driver_path)
    
    set(driver_src_suffix "(at32f45x_)*([a-z0-9]+)\.c$")

    #该file指令中*的作用：匹配任意数量的任意字符
    file(GLOB file_list
        RELATIVE "${driver_path}"
        "${driver_path}/*.c"
    )
    
    list(TRANSFORM file_list REPLACE "${driver_src_suffix}" "\\2")
    #output_list是函数参数名，在此需要用${}引用
    set(${output_list} ${file_list} PARENT_SCOPE)
endfunction()

if(NOT DRIVER_SRC_LIST)
    message(STATUS "Finding Driver...")
    generate_driver_list(DRIVER_SRC_LIST "${DRIVER_SRC_PATH}")
endif()

if(NOT DRIVER_INC_PATH)
    find_path(DRIVER_INC_PATH
        NAMES at32f45x_acc.h
        PATHS "${DRIVER_SRC_PATH}/../inc"
    )
endif()
message("Driver inc path: ${DRIVER_INC_PATH}")
set(DRIVER_INC "${DRIVER_INC_PATH}")

if(NOT TARGET DRIVER::INC)
    add_library(DRIVER::INC INTERFACE IMPORTED)
    target_include_directories(DRIVER::INC INTERFACE "${DRIVER_INC_PATH}")
endif()

message(STATUS "Driver libraries:")
foreach(DRIVER ${DRIVER_SRC_LIST})
    string(TOLOWER ${DRIVER} DRIVER_L)
    string(TOUPPER ${DRIVER} DRIVER_U)

    find_file(${DRIVER_U}_SRC
        NAMES at32f45x_${DRIVER_L}.c
        PATHS "${DRIVER_SRC_PATH}"
    )
    
    if(NOT DRIVER::${DRIVER_U})
        add_library(DRIVER::${DRIVER_U} INTERFACE IMPORTED)
        target_include_directories(DRIVER::${DRIVER_U} INTERFACE "${DRIVER_INC_PATH}")
        target_sources(DRIVER::${DRIVER_U} INTERFACE "${${DRIVER_U}_SRC}")
        message("                       DRIVER::${DRIVER_U}")
    endif()

    list(APPEND DRIVER_SOURCE "${${DRIVER_U}_SRC}")
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DRIVER
    REQUIRED_VARS DRIVER_SOURCE DRIVER_INC
    FOUND_VAR DRIVER_FOUND
    HANDLE_COMPONENTS
)



