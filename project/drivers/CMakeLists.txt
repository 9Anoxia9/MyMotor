cmake_minimum_required(VERSION 3.31)

if(NOT (TARGET at32_driver_wrap))
    add_library(at32_driver_wrap STATIC)

    target_include_directories(at32_driver_wrap PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_sources(at32_driver_wrap PRIVATE 
        at32_gpio.cpp
        at32_spi.cpp
    )

endif()
