#cmake版本
cmake_minimum_required(VERSION 3.31)

#设置工具链
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/gcc.cmake)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(main C ASM CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(ENABLE_MY_DEBUG "Enable Debug" ON)


find_package(CMSIS REQUIRED)
find_package(DRIVER REQUIRED)

add_executable(main main.cpp usb_descriptors.c)
set_target_properties(main PROPERTIES OUTPUT_NAME "MyMotor")

add_library(config INTERFACE)
target_include_directories(config INTERFACE
    ${CMAKE_SOURCE_DIR}
)


######################以下第三方
#ARM官方DSP包
add_subdirectory(../ThirdParty/CMSIS-DSP ${CMAKE_CURRENT_BINARY_DIR}/dsp)
target_link_libraries(CMSISDSP 
        PUBLIC AT32::F45::5
)


#FreeRTOS
set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING \"\")
set(FREERTOS_HEAP 4)

add_library(freertos_config INTERFACE)
target_link_libraries(freertos_config 
        INTERFACE config
)
add_subdirectory(../ThirdParty/FreeRTOS ${CMAKE_CURRENT_BINARY_DIR}/freertos)

target_link_libraries(freertos_kernel_port 
        PUBLIC AT32::F45::5
)

#TinyUSB
add_subdirectory(../ThirdParty/tinyusb-master ${CMAKE_CURRENT_BINARY_DIR}/tinyusb)
target_link_libraries(tinyusb PUBLIC 
        freertos_kernel
        CMSIS::DEVICE
        config
)

######################以上为第三方

######################以下为本项目源代码管理
add_subdirectory(board)
add_subdirectory(drivers)
add_subdirectory(usb_com)
add_subdirectory(motorcontrol)

if(ENABLE_MY_DEBUG)
    add_subdirectory(debug)
endif()


target_link_libraries(board PUBLIC 
    CMSIS::DEVICE
    DRIVER::CRM
    DRIVER::TMR
    DRIVER::ADC
    DRIVER::MISC
    DRIVER::ACC
    DRIVER::CRC
    at32_driver_wrap
    motorctrl
)
target_include_directories(board PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(at32_driver_wrap PUBLIC
    CMSIS::DEVICE
    DRIVER::GPIO
    DRIVER::EXINT
    DRIVER::SPI
    my_debug
)

target_link_libraries(usb PUBLIC
    tinyusb
    freertos_kernel
    board
    motorctrl
)

target_link_libraries(main PRIVATE
    Arm::NoSys
    Arm::Nano
    Arm::CppEmbedded
    board
    usb
)

target_link_libraries(my_debug PUBLIC
    usb
)

target_link_libraries(motorctrl PUBLIC
    CMSISDSP
    usb
    at32_driver_wrap
)


target_link_options(main 
    PRIVATE -Wl,-Map=main.map
)

print_size(main)
generate_ihex_file(main)