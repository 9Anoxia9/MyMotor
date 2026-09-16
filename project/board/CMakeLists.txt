if( NOT (TARGET board) )
    add_library(board board.cpp)
endif()

target_include_directories(board PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")

target_sources(board PRIVATE 
        src/crm.c 
        src/tim.c 
        src/gpio.c
        src/adc.c
        src/usb.c
        src/crc.c
        src/spi.c
)

