add_library(usb
    interface_usb.cpp
    interface_usart.cpp
    protocol.cpp
)

target_include_directories(usb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})