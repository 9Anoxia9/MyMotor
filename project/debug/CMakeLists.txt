if(NOT (TARGET debug))
    add_library(my_debug my_debug.cpp)

    target_include_directories(my_debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(my_debug PUBLIC ENABLE_ASSERT ENABLE_LOG)
endif()
