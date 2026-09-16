if(NOT TARGET motorctrl)

    add_library(motorctrl motor.cpp oscilloscope.cpp encoder.cpp utils.cpp
        axis.cpp foc.cpp openloop.cpp
    )
    target_include_directories(motorctrl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

endif()
